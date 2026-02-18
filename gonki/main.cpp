#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "camera.h"
#include "car.h"
#include "text.h"
#include "environnement.h"
#include "Road.h"

Camera cam;
Car car;

void processInput(GLFWwindow* window,float dt){
    float steerSpeed = 0.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) if (car.speed <= car.limitSpeed) car.speed += 5.0f * dt;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) if (car.speed > 0) car.speed -= 3.0f * dt;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) car.steering += steerSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) car.steering -= steerSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
    {
        float returnSpeed = 4.0f * dt;

        if (car.steering > 0.0f) car.steering = std::max(0.0f, car.steering - returnSpeed);
        else car.steering = std::min(0.0f, car.steering + returnSpeed);
    }


    car.steering = std::clamp(car.steering, -car.maxSteering, car.maxSteering);
}

int main(){
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1500, 800, "3D Example", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glEnable(GL_DEPTH_TEST);

    BuildFont();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glFrustum(cam.left, cam.right, cam.bottom, cam.top, cam.nearPlane, cam.farPlane);

    glMatrixMode(GL_MODELVIEW);

    double lastTime = glfwGetTime();
    double deltaTime = 0.0;

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        processInput(window,deltaTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
        drawSky();
        cam.setupCamera(car);
        drawGround(cam.cameraX,cam.cameraZ);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-2.0f, -2.0f);

        drawRoad();

        glDisable(GL_POLYGON_OFFSET_FILL);

        car.updatePos(deltaTime);
        car.draw();

        glDisable(GL_DEPTH_TEST);
        car.drawHud();
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

