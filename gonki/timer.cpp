#include <windows.h>
#include <GLFW/glfw3.h>
#include "timer.h"

void Timer::start() {
    isStarted = true;

    startTime = glfwGetTime();
}

float Timer::getCurrentTime() {
    if (!isStarted)
        return finalTime;

    return glfwGetTime() - startTime;
}

void Timer::stop() {
    finalTime = glfwGetTime() - startTime;
    isStarted = false;
}