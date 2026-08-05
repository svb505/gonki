#pragma once
#include <windows.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

void initLighting() {
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	GLfloat lightPos[] = { 0.0f, 2.0f,-10.0f, 1.0f }; //x,y,z,type 
	GLfloat lightAmbient[] = { 0.8f, 0.8f, 0.8f, 
		1.0f };//Background color r,g,b,a 
	GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f,
		1.0f };//Diffuse light r,g,b,a 
	GLfloat lightSpec[] = { 1.0f, 1.0f, 1.0f, 1.0f };//Specular reflection r,g,b,a

	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);
	glEnable(GL_DEPTH_TEST);
}

