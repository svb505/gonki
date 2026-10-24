#include "bounds.h"
#include "vec3.h"
#include <algorithm>

Bounds MakeAABB(Vec3 center, float width, float height, float depth) {
    Bounds box;

    box.min =
    {
        center.x - width / 2.0f,
        center.y - height / 2.0f,
        center.z - depth / 2.0f
    };

    box.max =
    {
        center.x + width / 2.0f,
        center.y + height / 2.0f,
        center.z + depth / 2.0f
    };

    return box;
}
bool checkAABBCollision(const Bounds& target,Vec3& pos) {
    return pos.x >= target.min.x && pos.x <= target.max.x &&
        pos.y >= target.min.y && pos.y <= target.max.y &&
        pos.z >= target.min.z && pos.z <= target.max.z;
}
bool RayIntersectsAABB(const Vec3& rayOrigin, const Vec3& rayDir,
    const Bounds& b, float& tNear, float& tFar)
{
    tNear = -FLT_MAX;
    tFar = FLT_MAX;

    // Для каждой оси
    for (int i = 0; i < 3; ++i) {
        float origin = (i == 0 ? rayOrigin.x : (i == 1 ? rayOrigin.y : rayOrigin.z));
        float dir = (i == 0 ? rayDir.x : (i == 1 ? rayDir.y : rayDir.z));
        float minB = (i == 0 ? b.min.x : (i == 1 ? b.min.y : b.min.z));
        float maxB = (i == 0 ? b.max.x : (i == 1 ? b.max.y : b.max.z));

        if (fabs(dir) < 1e-6f) {
            if (origin < minB || origin > maxB)
                return false;
        }
        else {
            float t1 = (minB - origin) / dir;
            float t2 = (maxB - origin) / dir;
            if (t1 > t2) std::swap(t1, t2);
            tNear = std::max(tNear, t1);
            tFar = std::min(tFar, t2);
            if (tNear > tFar) return false;
            if (tFar < 0) return false;
        }
    }
    return true;
}

