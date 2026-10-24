#pragma once
#include <cstdint>
#include "vec3.h"


struct Bounds {
    Vec3 min;
    Vec3 max;
};
struct checkCol {
    bool checked;
    uint32_t id;
};

Bounds MakeAABB(Vec3 center, float width, float height, float depth);
bool RayIntersectsAABB(const Vec3& rayOrigin, const Vec3& rayDir,
    const Bounds& b, float& tNear, float& tFar);
bool checkAABBCollision(const Bounds& target, Vec3& pos);

