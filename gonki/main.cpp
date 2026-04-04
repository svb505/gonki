#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>  
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <enet/enet.h>
#include "common.h"
#include <algorithm>
#include <unordered_map>
#include <cstdint>
#include <unordered_set>
#include <string>
#include "camera.h"
#include "car.h"
#include "text.h"
#include "environnement.h"
#include "Road.h"
#include "rank.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GUI.h"

Camera cam;
Car car;
CarState myCar{};
GUI gui;

uint32_t myId = 0;
std::unordered_map<uint32_t, CarState> otherCars;


void fpsCount(double& deltaTime, double& lastTime, float& fpsTimer, int& frames, float& fps) {
    double currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    fpsTimer += deltaTime;
    frames++;

    if (fpsTimer >= 1.0f) {
        fps = frames / fpsTimer;
        frames = 0;
        fpsTimer = 0.0f;
    }
}
void SendState(ENetPeer* peer){
    ClientStatePacket packet{};
    packet.type = PacketType::ClientState;
    packet.state = myCar;

    ENetPacket* p = enet_packet_create(&packet,sizeof(packet),ENET_PACKET_FLAG_UNSEQUENCED);

    enet_peer_send(peer, 0, p);
}
void processInput(GLFWwindow* window,float dt){
    float steerSpeed = 0.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) if (myCar.speed <= car.limitSpeed) myCar.speed += 5.0f * dt;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) if (myCar.speed > 0) myCar.speed -= 3.0f * dt;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) myCar.angle += steerSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) myCar.angle -= steerSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE){
        float returnSpeed = 4.0f * dt;

        if (car.steering > 0.0f) car.steering = std::max(0.0f, car.steering - returnSpeed);
        else car.steering = std::min(0.0f, car.steering + returnSpeed);
    }

    car.steering = std::clamp(car.steering, -car.maxSteering, car.maxSteering);
}
int main(){
    if (!glfwInit()){
        std::cout << "Failed to initialize GLFW\n";
        return -1;
    }
    GLFWwindow* window = glfwCreateWindow(1500, 800, "3D Example", NULL, NULL);
    if (!window){
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    enet_initialize();
    atexit(enet_deinitialize);

    ENetHost* client = enet_host_create(nullptr, 1, 2, 0, 0);

    ENetAddress address;
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 7777;

    ENetPeer* server = enet_host_connect(client, &address, 2, 0);
    ENetEvent event;

    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT){
        std::cout << "Connected to server\n";
    }
    else { std::cout << "Connection failed\n"; return 1; }

    myCar.x = 0;
    myCar.y = 0;
    myCar.speed = 0;
    myCar.angle = 0;

    int totalLaps = 3;
    bool readyToRace = false;

    glfwMakeContextCurrent(window);
    glEnable(GL_DEPTH_TEST);

    BuildFont();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glFrustum(cam.left, cam.right, cam.bottom, cam.top, cam.nearPlane, cam.farPlane);
    glMatrixMode(GL_MODELVIEW);

    double lastTime = glfwGetTime();
    double deltaTime = 0.0;
    float fpsTimer = 0.0f;
    int frames = 0;
    float fps = 0.0f;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImGui::StyleColorsDark();

    while (!glfwWindowShouldClose(window)){
        RaceResult rank = getRank(myCar, otherCars);

        fpsCount(deltaTime, lastTime, fpsTimer, frames, fps);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        gui.render(readyToRace,server,fps,myCar,totalLaps,rank);

        ImGui::Render();

        drawSky();
        cam.setupCamera(myCar);
        drawGround(cam.cameraX,cam.cameraZ);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-2.0f, -2.0f);

        drawRoad();

        std::unordered_map<uint32_t, CarState> allCars = otherCars;
        car.drawAllCars(allCars, car);

        glDisable(GL_POLYGON_OFFSET_FILL);

        myCar.x += std::cos(myCar.angle) * myCar.speed * deltaTime;
        myCar.z += -std::sin(myCar.angle) * myCar.speed * deltaTime;

        glPushMatrix();
        glTranslatef(myCar.x, myCar.y, myCar.z); 
        glRotatef(myCar.angle * 57.2958f, 0, 1, 0);
        car.draw();
        glPopMatrix();

        SendState(server);

        while (enet_host_service(client, &event, 1) > 0) {
            if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                PacketType type = static_cast<PacketType>(event.packet->data[0]);

                if (type == PacketType::Snapshot) {
                    auto* snap = (SnapshotPacket*)event.packet->data;

                    for (uint32_t i = 0; i < snap->count; i++) {
                        CarState& s = snap->cars[i];
                        if (s.id == myCar.id) {
                            float dx = s.x - myCar.x;float dz = s.z - myCar.z;
                            float distance = std::sqrt(dx * dx + dz * dz);

                            const float MAX_DESYNC = 2.0f; 
                            if (distance > MAX_DESYNC) {
                                myCar.x = s.x; myCar.y = s.y; myCar.z = s.z;
                                myCar.angle = s.angle; myCar.speed = s.speed;
                            }
                        }
                        else otherCars[s.id] = s;
                    }
                    std::unordered_set<uint32_t> ids;
                    for (uint32_t i = 0; i < snap->count; i++) ids.insert(snap->cars[i].id);
                    for (auto it = otherCars.begin(); it != otherCars.end(); ) {
                        if (!ids.count(it->first)) it = otherCars.erase(it);
                        else ++it;
                    }
                    readyToRace = snap->count >= 1;
                }

                enet_packet_destroy(event.packet);
            }
        }

        for (auto& [id, state] : otherCars) {
            int place = rank.places[id];
            std::string hudAll = "Place: " + std::to_string(place) + "/" + std::to_string(rank.allCars.size());
            RenderTextWorld(state.x, state.y + 2.5f, state.z, 1, 1, 1, hudAll.c_str());
        }

        if (readyToRace) processInput(window, deltaTime);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

