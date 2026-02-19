#include "Road.h"
#include <windows.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

std::vector<Checkpoint> checkpoints = {
    { {35, 0}, 7.0f },
    { {95, 0}, 7.0f },
    { {95, 35}, 7.0f },
    { {130, 35}, 7.0f },
    { {130, 95}, 7.0f },
    { {95, 95}, 7.0f },
    { {35, 130}, 7.0f },
    { {0, 95}, 7.0f },
    { {0, 35}, 7.0f }
};

Vec2 perpendicular(const Vec2& d) {
    return { -d.y, d.x };
}

Vec2 normalizeVec2(const Vec2& v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len == 0) return { 0, 0 };
    return { v.x / len, v.y / len };
}

void drawRoadStraight(const Vec2& start, const Vec2& end, const RoadStyle& style) {
    Vec2 dir = normalizeVec2({ end.x - start.x, end.y - start.y });
    Vec2 n = perpendicular(dir);

    float w = style.width * 0.5f;

    Vec2 v1 = { start.x + n.x * w, start.y + n.y * w };
    Vec2 v2 = { start.x - n.x * w, start.y - n.y * w };
    Vec2 v3 = { end.x - n.x * w, end.y - n.y * w };
    Vec2 v4 = { end.x + n.x * w, end.y + n.y * w };

    glColor3f(0.25f, 0.25f, 0.25f);
    glBegin(GL_TRIANGLES);
    glVertex3f(v1.x, ROAD_Y, v1.y);
    glVertex3f(v2.x, ROAD_Y, v2.y);
    glVertex3f(v3.x, ROAD_Y, v3.y);

    glVertex3f(v1.x, ROAD_Y, v1.y);
    glVertex3f(v3.x, ROAD_Y, v3.y);
    glVertex3f(v4.x, ROAD_Y, v4.y);
    glEnd();
}

void drawRoadTurn(const Vec2& center, float radius, float startAngle, float endAngle, const RoadStyle& style, int segments) {
    float innerR = radius - style.width * 0.5f;
    float outerR = radius + style.width * 0.5f;

    float step = (endAngle - startAngle) / segments;

    glColor3f(0.25f, 0.25f, 0.25f);
    glBegin(GL_TRIANGLES);

    for (int i = 0; i < segments; i++) {
        float a0 = startAngle + step * i;
        float a1 = startAngle + step * (i + 1);

        Vec2 i0 = { center.x + cos(a0) * innerR, center.y + sin(a0) * innerR };
        Vec2 o0 = { center.x + cos(a0) * outerR, center.y + sin(a0) * outerR };
        Vec2 i1 = { center.x + cos(a1) * innerR, center.y + sin(a1) * innerR };
        Vec2 o1 = { center.x + cos(a1) * outerR, center.y + sin(a1) * outerR };

        glVertex3f(i0.x, ROAD_Y, i0.y);
        glVertex3f(o0.x, ROAD_Y, o0.y);
        glVertex3f(o1.x, ROAD_Y, o1.y);

        glVertex3f(i0.x, ROAD_Y, i0.y);
        glVertex3f(o1.x, ROAD_Y, o1.y);
        glVertex3f(i1.x, ROAD_Y, i1.y);
    }

    glEnd();
}

void drawRoad() {
    RoadStyle style{ 4.0f };
    float R = 35.0f;
    float L = 60.0f;

    Vec2 p0 = { R, 0 };
    Vec2 p1 = { R + L, 0 };
    drawRoadStraight(p0, p1, style);

    Vec2 c1 = { p1.x, p1.y + R };
    drawRoadTurn(c1, R, -myPI / 2, 0.0f, style);
    Vec2 p2 = { p1.x + R, R };
    Vec2 p3 = { p1.x + R, R + L };
    drawRoadStraight(p2, p3, style);

    Vec2 c2 = { p3.x - R, p3.y };
    drawRoadTurn(c2, R, 0.0f, myPI / 2, style);
    Vec2 p4 = { p3.x - R, p3.y + R };
    Vec2 p5 = { R, p3.y + R };
    drawRoadStraight(p4, p5, style);
    Vec2 c3 = { R, p3.y };
    drawRoadTurn(c3, R, myPI / 2, myPI, style);
    Vec2 p6 = { 0, R + L };
    Vec2 p7 = { 0, R };
    drawRoadStraight(p6, p7, style);
    Vec2 c4 = { R, R };
    drawRoadTurn(c4, R, myPI, 3 * myPI / 2, style);

    glPointSize(10);
    glBegin(GL_POINTS);
    glColor3f(1, 0, 0);

    for (const auto& cp : checkpoints)
    {
        glVertex3f(cp.pos.x, 0.5f, cp.pos.y);
    }

    glEnd();

}
