#pragma once

class Car {	
public:
	float positions[3] = { 10.0f,0.01f,0.0f };
	float speed = 0.0f;
	float rotation = 0.0f;     
	float steering = 0.0f;
	float wheelBase = 2.5f;

	float maxSteering = 25.0f * 3.1415926f / 180.0f;;
	float color[3] = { 0.0f,0.0f,0.0f };
	float limitSpeed = 150.0f;

	void draw();
	void updatePos(float dt);
	void drawHud();
};