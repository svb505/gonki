#pragma once
#include <GL/gl.h>
#include <GLFW/glfw3.h>

class Timer {
private:
	float currentTime = 0.0f;
	float finalTime = 0.0f;

	bool isStarted = false;
public:
	void start();
	void stop();
};
