#pragma once
#include <map>

struct Info {
	float positions[3] = { 0.0f,0.0f,0.0f };
	float speed = 0.0f;
	float rotationX = 0.0f;
	float color[3] = { 0.0f,0.0f,0.0f };
};

class Bot{
private:
	std::map<int, Info> bots;
public:
	void moveAlgorithme();
	void generateBots(int count = 6);
	void drawBots();
};
