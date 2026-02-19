#include <windows.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include "car.h"
#include "text.h"
#include <algorithm>
#include <format>

void Car::updatePos(float dt)
{
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

void Car::drawHud() {
    int w, h;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &w, &h);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    std::string formatted_speed = std::format("Speed: {}", speed);
    std::string formatted_c = std::format("X: {} | Y: {} | Z: {}", positions[0], positions[1], positions[2]);
    RenderTextHUD(20.0f, h - 40.0f, 1, 1, 1, formatted_speed.c_str(), w, h);
    RenderTextHUD(20.0f, h - 20.0f, 1, 1, 1, formatted_c.c_str(), w, h);


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
            if (car.lap >= totalLaps) {
                car.speed = 0;
            }
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
    std::vector<float> ranks;
    ranks.push_back(computeRank(myCar, totalCheckpoints));
    for (auto& [id, car] : others) {
        ranks.push_back(computeRank(car, totalCheckpoints));
    }
    std::sort(ranks.begin(), ranks.end(), std::greater<float>());
    auto it = std::find(ranks.begin(), ranks.end(), computeRank(myCar, totalCheckpoints));
    return std::distance(ranks.begin(), it) + 1; 
}

