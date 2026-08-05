#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

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
#include <algorithm>
#include <unordered_map>
#include <cstdint>
#include <unordered_set>
#include <string>
#include <cstring>

#include "common.h"
#include "camera.h"
#include "car.h"
#include "text.h"
#include "environnement.h"
#include "Road.h"
#include "rank.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "server.h"
#include "GUI.h"
#include "input.h"
#include "variables.h"
#include "lightning.h"
#include "database.h"
#include "timer.h"

Camera cam;
Car car;
CarState myCar{};
GUI gui;
Timer timer;

uint32_t myId = 0;
std::unordered_map<uint32_t, CarState> otherCars;

std::string myMes = "";
std::vector<std::string> allMessages;

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
int main() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 16);
    GLFWwindow* window = glfwCreateWindow(1500, 800, "Race", NULL, NULL);
    if (!window) {
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

    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "Connected to server\n";
    }
    else { std::cout << "Connection failed\n"; return 1; }

    myCar.setupCar();

    glfwMakeContextCurrent(window);
    glEnable(GL_DEPTH_TEST);

    BuildFont();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glFrustum(cam.left, cam.right, cam.bottom, cam.top, cam.nearPlane, cam.farPlane);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);

    double lastTime = glfwGetTime();
    double deltaTime = 0.0;
    float fpsTimer = 0.0f;
    int frames = 0;
    float fps = 0.0f;

    ChatContext chatContext = { allMessages,myMes };

    initLighting();
    gui.setup(window);
    timer.start();

    float drag = 5.0f; // brake force

    if (!dbIsExists()) createDb();

    openDatabase();

    while (!glfwWindowShouldClose(window)){
        RaceResult rank = getRank(myCar, otherCars);

        fpsCount(deltaTime, lastTime, fpsTimer, frames, fps);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        gui.render(server,fps,myCar,timer,rank,chatContext,currentCarsCount);

        ImGui::Render();

        if (sendChat) {
            SendChat(server,myMes);
            sendChat = false;
        }

        drawSky();

        cam.setupCamera(myCar);

        drawGround(cam.cameraX,cam.cameraZ);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-2.0f, -2.0f);

        drawRoad();

        std::unordered_map<uint32_t, CarState> allCars = otherCars;
        
        car.drawAllCars(allCars, car);

        glDisable(GL_POLYGON_OFFSET_FILL);

        //Update the car withnout waiting server response
        if (myCar.speed > 0.0f){
            myCar.speed -= drag * deltaTime;
            myCar.speed = std::max(0.0f, myCar.speed);
        }

        myCar.x += std::cos(myCar.angle) * myCar.speed * deltaTime;
        myCar.z += -std::sin(myCar.angle) * myCar.speed * deltaTime;
            
        if (myCar.lap == TOTAL_LAPS && readyToRace) {
            readyToRace = false;

            timer.stop();

            saveDataForPlayer(timer.getCurrentTime());
        }
       

        glPushMatrix();
        glTranslatef(myCar.x, myCar.y, myCar.z);
        glRotatef(myCar.angle * 57.2958f, 0, 1, 0);
        
        car.draw();

        glPopMatrix();

        SendState(server,myCar);

        while (enet_host_service(client, &event, 1) > 0) {
            if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                PacketType type = *(PacketType*)event.packet->data;

                if (type == PacketType::ClientState) {
                    auto* init = (ClientStatePacket*)event.packet->data;

                    if (myCar.id == 0) myCar = init->state;
                }

                if (type == PacketType::Chat) {
                    auto* p = (ChatPacket*)event.packet->data;
                    allMessages.emplace_back(p->msg);
                }

                if (type == PacketType::Snapshot) {
                    auto* snap = (SnapshotPacket*)event.packet->data;

                    for (uint32_t i = 0; i < snap->count; i++) {
                        CarState& s = snap->cars[i];
                        if (s.id == myCar.id){
                            float dx = s.x - myCar.x;
                            float dz = s.z - myCar.z;

                            float distance = std::sqrt(dx * dx + dz * dz);

                            const float MAX_DESYNC = 2.0f;

                            if (distance > MAX_DESYNC) {
                                myCar.x = s.x;
                                myCar.y = s.y;
                                myCar.z = s.z;
                            }

                            myCar.angle = s.angle;
                            myCar.speed = s.speed;
                            myCar.lap = s.lap;
                        }
                        else otherCars[s.id] = s;
                    }

                    std::unordered_set<uint32_t> ids;

                    for (uint32_t i = 0; i < snap->count; i++) 
                        ids.insert(snap->cars[i].id);

                    //Deleting the cars who are disappeared from the server
                    for (auto it = otherCars.begin(); it != otherCars.end(); ) {
                        if (!ids.count(it->first)) it = otherCars.erase(it);
                        else ++it;
                    }

                    readyToRace = snap->count >= MIN_PLAYERS;

                    currentCarsCount = snap->count;
                }

                enet_packet_destroy(event.packet);
            }
        }

        //Updating others cars 
        for (auto& [id, state] : otherCars) {
            int place = rank.places[id];

            std::string hudAll = "Place: " + std::to_string(place) + "/" + std::to_string(rank.allCars.size());
            
            RenderTextWorld(state.x, state.y + 2.5f, state.z, 
                1, 1, 1, hudAll.c_str());
        }

        if (readyToRace && myCar.lap < TOTAL_LAPS) processInput(window, car, myCar, deltaTime);
        
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

