#define NOMINMAX
#include "windows.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "model.h"
#include <fstream>
#include <sstream>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <format>
#include <filesystem>

#include <cstdint>
#include <chrono>

namespace {
    constexpr uint32_t CACHE_MAGIC = 0x4D424A4F; // "OBJM"
    constexpr uint32_t CACHE_VERSION = 2;          // увеличивайте при смене формата

    template<class T>
    void WritePod(std::ofstream& f, const T& v) {
        f.write(reinterpret_cast<const char*>(&v), sizeof(T));
    }

    template<class T>
    bool ReadPod(std::ifstream& f, T& v) {
        f.read(reinterpret_cast<char*>(&v), sizeof(T));
        return static_cast<bool>(f);
    }

    void WriteString(std::ofstream& f, const std::string& s) {
        uint32_t len = static_cast<uint32_t>(s.size());
        WritePod(f, len);
        f.write(s.data(), len);
    }

    bool ReadString(std::ifstream& f, std::string& s) {
        uint32_t len = 0;
        if (!ReadPod(f, len) || len > (1u << 20)) return false;
        s.resize(len);
        f.read(s.data(), len);
        return static_cast<bool>(f);
    }

    // размер + время изменения файла (0, если файла нет)
    void FileStamp(const std::string& path, uint64_t& size, int64_t& time) {
        std::error_code ec;
        size = 0;
        time = 0;
        if (path.empty()) return;

        auto s = std::filesystem::file_size(path, ec);
        if (ec) return;
        size = s;

        auto t = std::filesystem::last_write_time(path, ec);
        if (!ec) time = static_cast<int64_t>(t.time_since_epoch().count());
    }
}

bool OBJModel::SaveCache(const std::string& objPath, const std::string& cachePath, const std::string& mtlPath) const {
    const std::string tmpPath = cachePath + ".tmp";
    std::ofstream f(tmpPath, std::ios::binary | std::ios::trunc);
    if (!f.is_open()) return false;

    uint64_t objSize, mtlSize;
    int64_t objTime, mtlTime;
    FileStamp(objPath, objSize, objTime);
    FileStamp(mtlPath, mtlSize, mtlTime);

    WritePod(f, CACHE_MAGIC);
    WritePod(f, CACHE_VERSION);
    WritePod(f, static_cast<uint32_t>(sizeof(Vertex)));
    WritePod(f, objSize);
    WritePod(f, objTime);
    WriteString(f, mtlPath);
    WritePod(f, mtlSize);
    WritePod(f, mtlTime);

    // вершины одним блоком
    WritePod(f, static_cast<uint64_t>(vertices.size()));
    f.write(reinterpret_cast<const char*>(vertices.data()),
        vertices.size() * sizeof(Vertex));

    // материалы
    WritePod(f, static_cast<uint32_t>(materials.size()));
    for (const auto& m : materials) {
        WriteString(f, m.name);
        WritePod(f, m.Ka);
        WritePod(f, m.Kd);
        WritePod(f, m.Ks);
        WritePod(f, m.Ns);
        WritePod(f, m.d);
        WriteString(f, m.texturePath);
    }

    // группы (материал хранится индексом)
    WritePod(f, static_cast<uint32_t>(materialGroups.size()));
    for (const auto& g : materialGroups) {
        int32_t idx = -1;
        for (size_t i = 0; i < materials.size(); i++) {
            if (materials[i].name == g.material.name) { idx = static_cast<int32_t>(i); break; }
        }
        WritePod(f, idx);
        WritePod(f, static_cast<uint64_t>(g.startVertex));
        WritePod(f, static_cast<uint64_t>(g.vertexCount));
    }

    WritePod(f, bounds.min);
    WritePod(f, bounds.max);

    // части модели (по строкам "o" в .obj) - для точной коллизии
    WritePod(f, static_cast<uint32_t>(parts.size()));
    for (const auto& p : parts) {
        WriteString(f, p.name);
        WritePod(f, p.bounds.min);
        WritePod(f, p.bounds.max);
    }

    f.close();
    if (!f) {
        std::error_code ec;
        std::filesystem::remove(tmpPath, ec);
        return false;
    }

    std::error_code ec;
    std::filesystem::rename(tmpPath, cachePath, ec);
    if (ec) {
        std::filesystem::remove(cachePath, ec);
        std::filesystem::rename(tmpPath, cachePath, ec);
    }
    return !ec;
}
bool OBJModel::LoadFromCache(const std::string& objPath, const std::string& cachePath) {
    std::ifstream f(cachePath, std::ios::binary);
    if (!f.is_open()) return false;

    auto fail = [&]() {
        vertices.clear();
        materials.clear();
        materialGroups.clear();
        parts.clear();
        return false;
        };

    uint32_t magic = 0, version = 0, vertexSize = 0;
    if (!ReadPod(f, magic) || magic != CACHE_MAGIC) return false;
    if (!ReadPod(f, version) || version != CACHE_VERSION) return false;
    if (!ReadPod(f, vertexSize) || vertexSize != sizeof(Vertex)) return false;

    // проверка актуальности .obj
    uint64_t cachedObjSize = 0, curObjSize = 0;
    int64_t cachedObjTime = 0, curObjTime = 0;
    if (!ReadPod(f, cachedObjSize) || !ReadPod(f, cachedObjTime)) return false;
    FileStamp(objPath, curObjSize, curObjTime);
    if (cachedObjSize != curObjSize || cachedObjTime != curObjTime) return false;

    // проверка актуальности .mtl
    std::string mtlPath;
    uint64_t cachedMtlSize = 0, curMtlSize = 0;
    int64_t cachedMtlTime = 0, curMtlTime = 0;
    if (!ReadString(f, mtlPath) || !ReadPod(f, cachedMtlSize) || !ReadPod(f, cachedMtlTime))
        return false;
    FileStamp(mtlPath, curMtlSize, curMtlTime);
    if (cachedMtlSize != curMtlSize || cachedMtlTime != curMtlTime) return false;

    // вершины
    uint64_t vertexCount = 0;
    if (!ReadPod(f, vertexCount) || vertexCount == 0 || vertexCount > (1ull << 31))
        return fail();
    vertices.resize(static_cast<size_t>(vertexCount));
    f.read(reinterpret_cast<char*>(vertices.data()), vertexCount * sizeof(Vertex));
    if (!f) return fail();

    // материалы
    uint32_t materialCount = 0;
    if (!ReadPod(f, materialCount) || materialCount > 100000) return fail();
    materials.clear();
    materials.reserve(std::max<uint32_t>(materialCount, 32));
    for (uint32_t i = 0; i < materialCount; i++) {
        Material m{};
        if (!ReadString(f, m.name) ||
            !ReadPod(f, m.Ka) || !ReadPod(f, m.Kd) || !ReadPod(f, m.Ks) ||
            !ReadPod(f, m.Ns) || !ReadPod(f, m.d) ||
            !ReadString(f, m.texturePath))
            return fail();
        m.textureID = 0;
        materials.push_back(std::move(m));
    }

    // группы
    uint32_t groupCount = 0;
    if (!ReadPod(f, groupCount) || groupCount > 1000000) return fail();
    materialGroups.clear();
    materialGroups.reserve(groupCount);
    for (uint32_t i = 0; i < groupCount; i++) {
        int32_t idx = -1;
        uint64_t start = 0, count = 0;
        if (!ReadPod(f, idx) || !ReadPod(f, start) || !ReadPod(f, count)) return fail();
        if (idx >= static_cast<int32_t>(materials.size())) return fail();
        if (start + count > vertices.size()) return fail();

        MaterialGroup g;
        if (idx >= 0) g.material = materials[idx];
        g.startVertex = static_cast<decltype(g.startVertex)>(start);
        g.vertexCount = static_cast<decltype(g.vertexCount)>(count);
        materialGroups.push_back(std::move(g));
    }

    if (!ReadPod(f, bounds.min) || !ReadPod(f, bounds.max)) return fail();

    uint32_t partCount = 0;
    if (!ReadPod(f, partCount) || partCount > 100000) return fail();
    parts.clear();
    parts.reserve(partCount);
    for (uint32_t i = 0; i < partCount; i++) {
        ObjPart part;
        if (!ReadString(f, part.name) ||
            !ReadPod(f, part.bounds.min) ||
            !ReadPod(f, part.bounds.max))
            return fail();
        parts.push_back(std::move(part));
    }

    // текстуры (GL-объекты в кэш не положишь — грузим из картинок)
    for (auto& m : materials) {
        if (!m.texturePath.empty())
            m.textureID = LoadTextureFromFile(m.texturePath);
    }
    for (auto& g : materialGroups) {
        for (const auto& m : materials) {
            if (m.name == g.material.name) { g.material.textureID = m.textureID; break; }
        }
    }

    return true;
}

GLuint OBJModel::LoadTextureFromFile(const std::string& filename) {
    int width, height, channels;

    stbi_set_flip_vertically_on_load(1);

    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 4);

    if (!data) return 0;

    stbi_set_flip_vertically_on_load(0);

    GLuint textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR
    );
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR
    );
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT
    );
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT
    );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data
    );

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    return textureID;
}
bool OBJModel::LoadMTL(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) return false;

    std::string directory;
    size_t slash = filename.find_last_of("/\\");

    if (slash != std::string::npos)
        directory = filename.substr(0, slash + 1);

    Material* current = nullptr;
    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);

        std::string type;
        ss >> type;

        if (type.empty() || type[0] == '#')
            continue;

        if (type == "newmtl") {
            std::string name;
            ss >> name;

            materials.push_back({});
            materials.back().name = name;
            current = &materials.back();
        }
        else if (!current) {
            continue;
        }
        else if (type == "Ka") {
            ss >> current->Ka[0]
                >> current->Ka[1]
                >> current->Ka[2];
        }
        else if (type == "Kd") {
            ss >> current->Kd[0]
                >> current->Kd[1]
                >> current->Kd[2];
        }
        else if (type == "Ks") {
            ss >> current->Ks[0]
                >> current->Ks[1]
                >> current->Ks[2];
        }
        else if (type == "Ns") {
            ss >> current->Ns;
        }
        else if (type == "d") {
            ss >> current->d;
        }
        else if (type == "Tr") {
            float transparency;
            ss >> transparency;
            current->d = 1.0f - transparency;
        }
        else if (type == "map_Kd") {
            std::string textureFile;
            std::getline(ss >> std::ws, textureFile);

            if (!textureFile.empty()) {
                current->texturePath = directory + textureFile;
                current->textureID =
                    LoadTextureFromFile(current->texturePath);
            }
        }
    }

    return true;
}
bool OBJModel::LoadOBJ(const std::string& filename, float scale) {
    vertices.clear();
    materials.clear();
    materialGroups.clear();
    parts.clear();

    this->scale = scale;

    const std::string cachePath = filename + ".cache";
    std::string mtlPath;

    if (LoadFromCache(filename, cachePath)) return true;

    std::string directory;

    size_t slash = filename.find_last_of("/\\");

    if (slash != std::string::npos)
        directory = filename.substr(0, slash + 1);

    std::ifstream file(filename);

    if (!file.is_open())  return false;

    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texcoords;

    const auto fileSize = std::filesystem::file_size(filename);

    const size_t estimatedFloats = static_cast<size_t>(fileSize / 4);

    positions.reserve(estimatedFloats / 2);
    normals.reserve(estimatedFloats / 4);
    texcoords.reserve(estimatedFloats / 4);

    vertices.reserve(fileSize / 4);
    materialGroups.reserve(32);
    materials.reserve(32);

    // (имя объекта, индекс первой вершины) для каждой строки "o"
    std::vector<std::pair<std::string, size_t>> partStarts;

    std::string line;

    while (std::getline(file, line)) {
        std::stringstream ss(line);

        std::string type;
        ss >> type;


        // POSITION
        if (type == "v") {
            float x, y, z;

            ss >> x >> y >> z;

            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }
        // UV
        else if (type == "vt") {
            float u, v;

            ss >> u >> v;

            texcoords.push_back(u);
            texcoords.push_back(v);
        }
        // NORMAL
        else if (type == "vn") {
            float x, y, z;

            ss >> x >> y >> z;

            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);
        }
        // OBJECT
        else if (type == "o") {
            std::string objName;
            std::getline(ss >> std::ws, objName);

            partStarts.emplace_back(objName, vertices.size());
        }
        //MTL
        else if (type == "mtllib") {
            std::string mtlFile;
            ss >> mtlFile;

            mtlPath = directory + mtlFile;
            LoadMTL(mtlPath);
        }
        else if (type == "usemtl") {
            std::string materialName;
            ss >> materialName;

            MaterialGroup group;

            group.material.name = materialName;
            group.startVertex = vertices.size();

            materialGroups.push_back(group);
        }
        // FACE
        else if (type == "f") {
            std::vector<std::string> face;
            std::string value;

            while (ss >> value) face.push_back(value);

            if (face.size() < 3) continue;

            auto ParseVertex = [&](const std::string& str) {
                Vertex v{};

                int p = 0;
                int t = 0;
                int n = 0;

                // -------------------------------------------------
                // v
                // -------------------------------------------------
                if (str.find('/') == std::string::npos) {

                    sscanf_s(str.c_str(), "%d", &p);

                    p--;

                    v.x = positions[p * 3 + 0];
                    v.y = positions[p * 3 + 1];
                    v.z = positions[p * 3 + 2];

                    v.u = 0.0f;
                    v.v = 0.0f;

                    // Нормаль пока отсутствует
                    v.nx = 0.0f;
                    v.ny = 1.0f;
                    v.nz = 0.0f;
                }

                // -------------------------------------------------
                // v//vn
                // -------------------------------------------------
                else if (str.find("//") != std::string::npos) {

                    sscanf_s(str.c_str(), "%d//%d", &p, &n);

                    p--;
                    n--;

                    v.x = positions[p * 3 + 0];
                    v.y = positions[p * 3 + 1];
                    v.z = positions[p * 3 + 2];

                    v.u = 0.0f;
                    v.v = 0.0f;

                    v.nx = normals[n * 3 + 0];
                    v.ny = normals[n * 3 + 1];
                    v.nz = normals[n * 3 + 2];
                }

                // -------------------------------------------------
                // v/vt/vn
                // -------------------------------------------------
                else {

                    sscanf_s(str.c_str(), "%d/%d/%d", &p, &t, &n);

                    p--;
                    t--;
                    n--;

                    v.x = positions[p * 3 + 0];
                    v.y = positions[p * 3 + 1];
                    v.z = positions[p * 3 + 2];

                    v.u = texcoords[t * 2 + 0];
                    v.v = texcoords[t * 2 + 1];

                    v.nx = normals[n * 3 + 0];
                    v.ny = normals[n * 3 + 1];
                    v.nz = normals[n * 3 + 2];
                }

                return v;
                };


            // TRIANGULATION
            // 1 2 3
            vertices.push_back(ParseVertex(face[0]));
            vertices.push_back(ParseVertex(face[1]));
            vertices.push_back(ParseVertex(face[2]));

            for (size_t i = 3; i < face.size(); i++) {
                vertices.push_back(ParseVertex(face[0]));
                vertices.push_back(ParseVertex(face[i - 1]));
                vertices.push_back(ParseVertex(face[i]));
            }
        }
    }

    for (auto& group : materialGroups) {
        for (const auto& material : materials) {
            if (material.name == group.material.name) {
                group.material = material;
                break;
            }
        }
    }
    if (materialGroups.empty() && !vertices.empty()) {
        MaterialGroup group;

        group.startVertex = 0;
        group.vertexCount = vertices.size();

        materialGroups.push_back(group);
    }

    for (size_t i = 0; i < materialGroups.size(); i++) {
        if (i + 1 < materialGroups.size()) {
            materialGroups[i].vertexCount =
                materialGroups[i + 1].startVertex -
                materialGroups[i].startVertex;
        }
        else {
            materialGroups[i].vertexCount =
                vertices.size() -
                materialGroups[i].startVertex;
        }
    }

    // границы каждого объекта отдельно
    for (size_t i = 0; i < partStarts.size(); i++) {
        const size_t begin = partStarts[i].second;
        const size_t end = (i + 1 < partStarts.size())
            ? partStarts[i + 1].second
            : vertices.size();

        if (begin >= end) continue;

        ObjPart part;
        part.name = partStarts[i].first;
        part.bounds.min = { vertices[begin].x, vertices[begin].y, vertices[begin].z };
        part.bounds.max = part.bounds.min;

        for (size_t k = begin; k < end; k++) {
            const Vertex& v = vertices[k];

            part.bounds.min = { std::min(part.bounds.min.x, v.x),
                                std::min(part.bounds.min.y, v.y),
                                std::min(part.bounds.min.z, v.z) };

            part.bounds.max = { std::max(part.bounds.max.x, v.x),
                                std::max(part.bounds.max.y, v.y),
                                std::max(part.bounds.max.z, v.z) };
        }

        parts.push_back(std::move(part));
    }

    bounds.min = { vertices[0].x, vertices[0].y, vertices[0].z };
    bounds.max = { vertices[0].x, vertices[0].y, vertices[0].z };

    for (const auto& v : vertices) {
        bounds.min = { std::min(bounds.min.x, v.x),
                       std::min(bounds.min.y, v.y),
                       std::min(bounds.min.z, v.z) };


        bounds.max = { std::max(bounds.max.x, v.x),
                       std::max(bounds.max.y, v.y),
                       std::max(bounds.max.z, v.z)
        };
    }

    if (!SaveCache(filename, cachePath, mtlPath)) std::cout << "Cannot write cache: {}" << cachePath;


    return true;
}
void OBJModel::CreateVBO() {
    if (vertices.empty()) return;

    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void OBJModel::Draw(Vec3 pos, float yaw, float pitch) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glPushMatrix();

    glTranslatef(pos.x, pos.y, pos.z);
    glRotatef(yaw, 0, 1, 0);
    glRotatef(pitch, 1, 0, 0);
    glScalef(scale, scale, scale);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);


    // POSITION
    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (void*)0);

    // NORMAL
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));

    // UV
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));


    // DRAW
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    for (const auto& group : materialGroups) {
        GLuint textureID = group.material.textureID;

        // Если у материала нет собственной текстуры,
        // используем общую текстуру модели.
        if (textureID == 0)
            textureID = texture;

        glBindTexture(GL_TEXTURE_2D, textureID);

        GLfloat ambient[] = {
            group.material.Ka[0],
            group.material.Ka[1],
            group.material.Ka[2],
            1.0f
        };

        GLfloat diffuse[] = {
            group.material.Kd[0],
            group.material.Kd[1],
            group.material.Kd[2],
            group.material.d
        };

        GLfloat specular[] = {
            group.material.Ks[0],
            group.material.Ks[1],
            group.material.Ks[2],
            1.0f
        };

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);

        glMaterialf(
            GL_FRONT_AND_BACK,
            GL_SHININESS,
            group.material.Ns
        );

        glDrawArrays(
            GL_TRIANGLES,
            group.startVertex,
            group.vertexCount
        );
    }

    // DISABLE

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glPopMatrix();
};
OBJModel::~OBJModel() {
    if (vbo)
        glDeleteBuffers(1, &vbo);

    if (texture)
        glDeleteTextures(1, &texture);

    for (auto& material : materials) {
        if (material.textureID) {
            glDeleteTextures(1, &material.textureID);
            material.textureID = 0;
        }
    }
}
bool OBJModel::LoadTexture(const std::string& filename) {
    int width;
    int height;
    int channels;

    stbi_set_flip_vertically_on_load(1);

    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 4);

    if (!data) return false;

    stbi_set_flip_vertically_on_load(0);

    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    return true;
}

