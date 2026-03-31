#include "rank.h"
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "common.h"

RaceResult getRank(CarState& myCar, std::unordered_map<uint32_t, CarState>& otherCars) {
    std::vector<CarState> all;

    all.push_back(myCar);

    for (auto& [id, state] : otherCars) all.push_back(state);

    std::sort(all.begin(), all.end(), [](const CarState& a, const CarState& b) {
        if (a.lap != b.lap) return a.lap > b.lap;
        if (a.lastCheckpoint != b.lastCheckpoint) return a.lastCheckpoint > b.lastCheckpoint;
        return a.progress > b.progress;
    });

    std::unordered_map<uint32_t, int> places;

    for (size_t i = 0; i < all.size(); ++i) places[all[i].id] = static_cast<int>(i) + 1;

    return { places,all };
}