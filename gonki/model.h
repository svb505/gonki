#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>
#include "vec3.h"
#include "bounds.h"

struct Vertex {
    float x, y, z;       // position
    float nx, ny, nz;    // normal
    float u, v;          // UV
};
struct Material {
    std::string name;

    float Ka[3] = { 1.0f, 1.0f, 1.0f };
    float Kd[3] = { 1.0f, 1.0f, 1.0f };
    float Ks[3] = { 1.0f, 1.0f, 1.0f };

    float Ns = 0.0f;
    float d = 1.0f;

    std::string texturePath;
    GLuint textureID = 0;
};

struct ObjPart {
    std::string name;
    Bounds bounds;
};

struct MaterialGroup {
    Material material;
    size_t startVertex = 0;
    size_t vertexCount = 0;
};

class OBJModel {
private:
    std::vector<Vertex> vertices;
    std::vector<MaterialGroup> materialGroups;
    std::vector<Material> materials;

    GLuint vbo = 0;
    GLuint texture = 0;
    bool LoadFromCache(const std::string& objPath, const std::string& cachePath);
    bool SaveCache(const std::string& objPath, const std::string& cachePath,
        const std::string& mtlPath) const;

public:
    std::vector<ObjPart> parts;
    Bounds bounds;
    float scale = 0.0f;

    GLuint LoadTextureFromFile(const std::string& filename);
    bool LoadMTL(const std::string& filename);
    bool LoadOBJ(const std::string& filename, float scale);
    bool LoadTexture(const std::string& filename);

    void CreateVBO();
    void Draw(Vec3 pos, float yaw, float pitch);

    ~OBJModel();
};
