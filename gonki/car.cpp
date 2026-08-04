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
    glPushMatrix();

    glTranslatef(positions.x, positions.y, positions.z);
    glRotatef(rotation, 0, 1, 0);

    // Body
    glPushMatrix();

    glScalef(2.5f, 0.35f, 1.5f);

    glColor3f(0.0f, 0.7f, 0.1f);

    glBegin(GL_QUADS);

    // front
    glVertex3f(-1, -1, 1);
    glVertex3f(1, -1, 1);
    glVertex3f(1, 1, 1);
    glVertex3f(-1, 1, 1);

    // back
    glVertex3f(-1, -1, -1);
    glVertex3f(-1, 1, -1);
    glVertex3f(1, 1, -1);
    glVertex3f(1, -1, -1);

    // left
    glVertex3f(-1, -1, -1);
    glVertex3f(-1, -1, 1);
    glVertex3f(-1, 1, 1);
    glVertex3f(-1, 1, -1);

    // right
    glVertex3f(1, -1, -1);
    glVertex3f(1, 1, -1);
    glVertex3f(1, 1, 1);
    glVertex3f(1, -1, 1);

    // top
    glVertex3f(-1, 1, -1);
    glVertex3f(-1, 1, 1);
    glVertex3f(1, 1, 1);
    glVertex3f(1, 1, -1);

    // bottom
    glVertex3f(-1, -1, -1);
    glVertex3f(1, -1, -1);
    glVertex3f(1, -1, 1);
    glVertex3f(-1, -1, 1);

    glEnd();

    glPopMatrix();


    // Cabine
    glPushMatrix();

    glTranslatef(0, 0.55f, -0.2f);

    glColor3f(0.08f, 0.08f, 0.12f);

    glBegin(GL_QUADS);


    // front windshield
    glVertex3f(-1.2f, 0, 0.7f);
    glVertex3f(1.2f, 0, 0.7f);
    glVertex3f(1.2f, 1.0f, 0.3f);
    glVertex3f(-1.2f, 1.0f, 0.3f);


    // roof
    glVertex3f(-1.2f, 1.0f, 0.3f);
    glVertex3f(1.2f, 1.0f, 0.3f);
    glVertex3f(1.0f, 1.0f, -0.8f);
    glVertex3f(-1.0f, 1.0f, -0.8f);


    // rear window
    glVertex3f(-1.0f, 1.0f, -0.8f);
    glVertex3f(1.0f, 1.0f, -0.8f);
    glVertex3f(1.2f, 0, -0.8f);
    glVertex3f(-1.2f, 0, -0.8f);


    // left
    glVertex3f(-1.2f, 0, 0.7f);
    glVertex3f(-1.2f, 1.0f, 0.3f);
    glVertex3f(-1.0f, 1.0f, -0.8f);
    glVertex3f(-1.2f, 0, -0.8f);


    // right
    glVertex3f(1.2f, 0, 0.7f);
    glVertex3f(1.2f, 0, -0.8f);
    glVertex3f(1.0f, 1.0f, -0.8f);
    glVertex3f(1.2f, 1.0f, 0.3f);


    glEnd();

    glPopMatrix();


    // Bumpers
    glPushMatrix();

    glTranslatef(0.0f, -0.25f, 1.52f);

    glScalef(2.3f, 0.12f, 0.06f);

    glColor3f(0.05f, 0.05f, 0.05f);

    drawCube();

    glPopMatrix();

    glPushMatrix();

    glTranslatef(0.0f, -0.25f, -1.52f);

    glScalef(2.3f, 0.12f, 0.06f);

    glColor3f(0.05f, 0.05f, 0.05f);

    drawCube();

    glPopMatrix();


    // Headlights
    glPushMatrix();

    glTranslatef(-1.7f, 0.0f, 1.52f);

    glScalef(0.15f, 0.1f, 0.05f);

    glColor3f(1.0f, 1.0f, 0.85f);

    drawCube();

    glPopMatrix();

    glPushMatrix();

    glTranslatef(1.7f, 0.0f, 1.52f);

    glScalef(0.15f, 0.1f, 0.05f);

    glColor3f(1.0f, 1.0f, 0.85f);

    drawCube();

    glPopMatrix();


    // Taillights
    glPushMatrix();

    glTranslatef(-1.7f, 0.0f, -1.52f);

    glScalef(0.15f, 0.1f, 0.05f);

    glColor3f(0.8f, 0.05f, 0.05f);

    drawCube();

    glPopMatrix();

    glPushMatrix();

    glTranslatef(1.7f, 0.0f, -1.52f);

    glScalef(0.15f, 0.1f, 0.05f);

    glColor3f(0.8f, 0.05f, 0.05f);

    drawCube();

    glPopMatrix();


    // Wheels
    auto drawWheel = [&](float x, float z, bool front) {
        glPushMatrix();

        glTranslatef(x, -0.55f, z);


        if (front) glRotatef(wheelAngle, 0, 1, 0);

        glRotatef(90, 0, 1, 0);

        glColor3f(0.02f, 0.02f, 0.02f);

        drawCylinder(0.45f, 0.35f, 32);

        glColor3f(0.75f, 0.75f, 0.75f);

        drawCylinder(0.22f, 0.37f, 24);

        glPopMatrix();
        };


    // front wheels
    drawWheel(-2.2f, 0.9f, true);
    drawWheel(2.2f, 0.9f, true);


    // back wheels
    drawWheel(-2.2f, -0.9f, false);
    drawWheel(2.2f, -0.9f, false);

    glPopMatrix();
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
        glTranslatef(state.x, 0.0f, state.z);
        glRotatef(state.angle * 57.2958f, 0, 1, 0);
        car.draw();
        glPopMatrix();
    }
}
