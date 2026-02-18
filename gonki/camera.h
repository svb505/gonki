#pragma once
#include <cmath>
#include "car.h"
#include <GL/glu.h>

class Camera {
public:
    float fov = 45.0f;
    float aspect = 800.0f / 600.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    float top = tan(fov * 3.1415926 / 360.0f) * nearPlane;
    float bottom = -top;
    float right = top * aspect;
    float left = -right;
    float camDistance = 6.0f;
    float camHeight = 2.5f;
    float cameraX = 0.0f;
    float cameraY = 0.0f;
    float cameraZ = 0.0f;
    float cameraYaw = 0.0f;

    void setupCamera(Car& car)
    {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        float forwardX = cos(car.rotation);
        float forwardZ = -sin(car.rotation); 

        float targetX = car.positions[0] - forwardX * camDistance;
        float targetY = car.positions[1] + camHeight;
        float targetZ = car.positions[2] - forwardZ * camDistance;

        float smooth = 0.15f;
        cameraX += (targetX - cameraX) * smooth;
        cameraY += (targetY - cameraY) * smooth;
        cameraZ += (targetZ - cameraZ) * smooth;

        float lookX = car.positions[0] + forwardX * 5.0f;
        float lookY = car.positions[1] + 1.0f;
        float lookZ = car.positions[2] + forwardZ * 5.0f;

        gluLookAt(
            cameraX, cameraY, cameraZ,
            lookX, lookY, lookZ,
            0, 1, 0
        );
    }
};
	
