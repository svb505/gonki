#include <windows.h>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cmath>
#include "car.h"
#include "text.h"
#include <algorithm>
#include <format>
#include <string>
#include "rank.h"
#include "model.h"

void Car::init() {
    carModel.LoadOBJ("models//car//car.obj", 0.7f);
    carModel.CreateVBO();
}
const float Car::getReductionCoef() {
    return reductionCoef;
}
void Car::setSteering(float steering) {
    this->steering = steering;
}
float Car::getLimitSpeed() {
    return limitSpeed;
}
float Car::getMaxSteering() {
    return maxSteering;
}
float Car::getSteering() {
    return steering;
}

void Car::updatePos(float dt){
    if (fabs(speed) < 0.001f) return;

    float turnRadius = wheelBase / tan(steering);
    float angularVelocity = speed / turnRadius;

    rotation += angularVelocity * dt;

    positions.x += cos(rotation) * speed * dt;
    positions.z += -sin(rotation) * speed * dt;
}
void Car::drawCylinder(float radius, float width, int segments) {
    float half = width / 2.0f;

    glBegin(GL_QUAD_STRIP);

    for (int i = 0; i <= segments; i++) {
        float a = 2.0f * 3.1415926f * i / segments;

        float y = cos(a) * radius;
        float z = sin(a) * radius;


        glVertex3f(-half, y, z);
        glVertex3f(half, y, z);
    }

    glEnd();


    glBegin(GL_TRIANGLE_FAN);

    glVertex3f(-half, 0, 0);

    for (int i = 0; i <= segments; i++) {
        float a = 2.0f * 3.1415926f * i / segments;

        glVertex3f(
            -half,
            cos(a) * radius,
            sin(a) * radius
        );
    }

    glEnd();


    glBegin(GL_TRIANGLE_FAN);

    glVertex3f(half, 0, 0);

    for (int i = 0; i <= segments; i++) {
        float a = 2.0f * 3.1415926f * i / segments;

        glVertex3f(half, cos(a) * radius, sin(a) * radius);
    }

    glEnd();
}
void Car::drawCube() {
    glBegin(GL_QUADS);

    // Front
    glVertex3f(-1, -1, 1);
    glVertex3f(1, -1, 1);
    glVertex3f(1, 1, 1);
    glVertex3f(-1, 1, 1);

    // Back
    glVertex3f(-1, -1, -1);
    glVertex3f(-1, 1, -1);
    glVertex3f(1, 1, -1);
    glVertex3f(1, -1, -1);

    // Left
    glVertex3f(-1, -1, -1);
    glVertex3f(-1, -1, 1);
    glVertex3f(-1, 1, 1);
    glVertex3f(-1, 1, -1);

    // Right
    glVertex3f(1, -1, -1);
    glVertex3f(1, 1, -1);
    glVertex3f(1, 1, 1);
    glVertex3f(1, -1, 1);

    // Top
    glVertex3f(-1, 1, -1);
    glVertex3f(-1, 1, 1);
    glVertex3f(1, 1, 1);
    glVertex3f(1, 1, -1);

    // Bottom
    glVertex3f(-1, -1, -1);
    glVertex3f(1, -1, -1);
    glVertex3f(1, -1, 1);
    glVertex3f(-1, -1, 1);

    glEnd();
}
void Car::draw() {
    carModel.Draw({ positions.x, positions.y, positions.z }, rotation, 0.0f);
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
        carModel.Draw({state.x, 0.2f, state.z},state.angle,0.0f);
    }
}
