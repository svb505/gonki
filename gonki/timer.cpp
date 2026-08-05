#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "timer.h"

void Timer::start() {
	isStarted = true;

	if (isStarted) currentTime = glfwGetTime();
}
void Timer::stop() {
	isStarted = false;

	finalTime = currentTime;
}