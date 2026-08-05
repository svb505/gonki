#pragma once

class Timer {
private:
	float startTime = 0.0f;
	float finalTime = 0.0f;

	bool isStarted = false;
public:
	float getCurrentTime();

	void start();
	void stop();
};
