#pragma once

#include <GL/glew.h>

typedef struct {
	GLuint program;
	GLuint vertexShader;
	GLuint fragmentShader;
} Shader;

Shader* createShader(char* vertexShaderPath, char* fragmentShaderPath);
void cleanShader(Shader* shader);