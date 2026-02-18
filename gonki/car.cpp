#include <windows.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include "car.h"
#include "text.h"
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
