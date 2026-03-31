#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "common.h"

struct RaceResult {
    std::unordered_map<uint32_t, int> places;  
    std::vector<CarState> allCars;             
};
RaceResult getRank(CarState& myCar, std::unordered_map<uint32_t, CarState>& otherCars);
