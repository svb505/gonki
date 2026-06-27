#pragma once
#include <GLFW/glfw3.h>
#include "car.h"

bool isPressed(GLFWwindow* window,int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}
bool isReleased(GLFWwindow* window,int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}
void processInput(GLFWwindow* window, Car& car,CarState& carState,float dt) {
    float steerSpeed = 0.5f;

    if (isPressed(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);

    if (isPressed(window, GLFW_KEY_W)) if (carState.speed <= car.limitSpeed)
        carState.speed += 5.0f * dt;
    if (isPressed(window, GLFW_KEY_S)) if (carState.speed > 0)
        carState.speed -= 3.0f * dt;
    if (isPressed(window, GLFW_KEY_A)) carState.angle += steerSpeed * dt;
    if (isPressed(window, GLFW_KEY_D)) carState.angle -= steerSpeed * dt;
    if (isReleased(window, GLFW_KEY_A) && isReleased(window, GLFW_KEY_D)) {
        float returnSpeed = 4.0f * dt;

        if (car.steering > 0.0f) car.steering = std::max(0.0f, car.steering - returnSpeed);
        else car.steering = std::min(0.0f, car.steering + returnSpeed);
    }

    car.steering = std::clamp(car.steering, -car.maxSteering, car.maxSteering);
}