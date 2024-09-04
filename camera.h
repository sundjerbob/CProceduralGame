#pragma once

#include <GL/glew.h>

typedef struct {
	GLfloat* position;
	float* rotation;
	float* targetLocation;
	float* targetOffset;
	float fov;
	float aspect;
	float near;
	float far;
	float view[16];
	float projection[16];
} Camera;

Camera* createCamera(float* targetLocation, float* targetOffset, float fov, float aspect, float near, float far);
void updateCamera(Camera* camera);