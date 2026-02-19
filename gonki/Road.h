#pragma once
#include <cmath>
#include <numbers>
#include <vector>

inline constexpr float myPI = std::numbers::pi_v<float>;
inline constexpr float ROAD_Y = 0.001f;

struct Vec2 {
    float x, y;
};

struct RoadStyle {
    float width;
};

struct Checkpoint {
    Vec2 pos;
    float radius;
};

Vec2 perpendicular(const Vec2& d);
Vec2 normalizeVec2(const Vec2& v);

extern std::vector<Checkpoint> checkpoints;

void drawRoadStraight(const Vec2& start, const Vec2& end, const RoadStyle& style);
void drawRoadTurn(const Vec2& center, float radius, float startAngle, float endAngle, const RoadStyle& style, int segments = 32);
void drawRoad();
