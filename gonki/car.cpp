#include <windows.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include "car.h"
#include "text.h"
#include <algorithm>
#include <format>
#include <string>
#include "rank.h"

void Car::updatePos(float dt){
    if (fabs(speed) < 0.001f) return;

    float turnRadius = wheelBase / tan(steering);
    float angularVelocity = speed / turnRadius;

    rotation += angularVelocity * dt;

    positions[0] += cos(rotation) * speed * dt;
    positions[2] += -sin(rotation) * speed * dt;
}
void Car::draw() {
    float step = 0.6f;

    glPushMatrix();

    glTranslatef(positions[0], positions[1], positions[2]);
    glRotatef(1, 0, 1, 0);

    glBegin(GL_QUADS);

    // Front
    glColor3f(0, 1, 0);
    glVertex3f(-1, -step, 1);
    glVertex3f(1, -step, 1);
    glVertex3f(1, step, 1);
    glVertex3f(-1, step, 1);

    // Back
    glColor3f(0, 1, 0);
    glVertex3f(1, -step, -1);
    glVertex3f(-1, step, -1);
    glVertex3f(1, step, -1);
    glVertex3f(1, -step, -1);

    // Left
    glColor3f(0, 1, 0.7);
    glVertex3f(-1, -step, -1);
    glVertex3f(-1, -step, 1);
    glVertex3f(-1, step, 1);
    glVertex3f(-1, step, -1);

    // Right
    glColor3f(0, 1, 0.7);
    glVertex3f(1, -step, -1);
    glVertex3f(1, step, -1);
    glVertex3f(1, step, 1);
    glVertex3f(1, -step, 1);

    // Top
    glColor3f(0, 1, 0);
    glVertex3f(-1, step, -1);
    glVertex3f(-1, step, 1);
    glVertex3f(1, step, 1);
    glVertex3f(1, step, -1);

    // Bottom
    glColor3f(0, 1, 0);
    glVertex3f(-1, -step, -1);
    glVertex3f(1, -step, -1);
    glVertex3f(1, -step, 1);
    glVertex3f(-1, -step, 1);

    glEnd();

    glPopMatrix();
}
void Car::drawHud(CarState& myCar, std::unordered_map<uint32_t, CarState>& otherCars,int totLaps, RaceResult& rank) {
    int w, h;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &w, &h);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int myPlace = rank.places[myCar.id];
    
    std::string hud = "Lap: " + std::to_string(myCar.lap + 1) + "/" + std::to_string(totLaps) +
        " Place: " + std::to_string(myPlace) + "/" + std::to_string(rank.allCars.size());
    std::string formatted_speed = std::format("Speed: {:.1f}", myCar.speed);

    RenderTextHUD(20.0f, h - 40.0f, 1, 1, 1, formatted_speed.c_str(), w, h);
    RenderTextHUD(10, 10, 1, 1, 1, hud.c_str(), 1500, 800);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
void Car::updateProgress(CarState& car, const std::vector<Checkpoint>& checkpoints, int totalLaps) {
    int next = (car.lastCheckpoint + 1) % checkpoints.size();
    Vec2 cp = checkpoints[next].pos;

    float dx = car.x - cp.x;
    float dz = car.z - cp.y; 
    float dist = std::sqrt(dx * dx + dz * dz);

    if (dist < checkpoints[next].radius) {
        car.lastCheckpoint = next;
        if (next == 0) { 
            car.lap++;
            if (car.lap >= totalLaps) car.speed = 0;
        }
    }

    Vec2 cpNext = checkpoints[(next + 1) % checkpoints.size()].pos;
    Vec2 dir = { cpNext.x - cp.x, cpNext.y - cp.y };
    Vec2 toCar = { car.x - cp.x, car.z - cp.y };
    float lenDir = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    car.progress = ((toCar.x * dir.x + toCar.y * dir.y) / (lenDir * lenDir));
    car.progress = std::clamp(car.progress, 0.0f, 1.0f);
}
float Car::computeRank(const CarState& car, int totalCheckpoints) {
    return car.lap * totalCheckpoints + car.lastCheckpoint + car.progress;
}
int Car::getPlayerPlace(const CarState& myCar, const std::unordered_map<uint32_t, CarState>& others, int totalCheckpoints) {
    float myRank = computeRank(myCar, totalCheckpoints);
    int place = 1;

    for (const auto& [id, car] : others) if (computeRank(car, totalCheckpoints) > myRank) place++;

    return place;
}
void Car::drawAllCars(std::unordered_map<uint32_t, CarState>& allCars,Car& car) {
    for (auto& [id, state] : allCars) {
        glPushMatrix();
        glTranslatef(state.x, 0.0f, state.y);
        glRotatef(state.angle * 57.2958f, 0, 1, 0);
        car.draw();
        glPopMatrix();
    }
}
