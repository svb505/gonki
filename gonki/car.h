#pragma once
#include "common.h"
#include "Road.h"
#include <vector>
#include <unordered_map>
#include "rank.h"

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
	void drawAllCars(std::unordered_map<uint32_t, CarState>& allCars, Car& car);
	void updatePos(float dt);
	void drawHud(CarState& myCar, std::unordered_map<uint32_t, CarState>& otherCars,int totLaps,RaceResult& rank);
	void updateProgress(CarState& car, const std::vector<Checkpoint>& checkpoints, int totalLaps);
	float computeRank(const CarState& car, int totalCheckpoints);
	int getPlayerPlace(const CarState& myCar, const std::unordered_map<uint32_t, CarState>& others, int totalCheckpoints);
};