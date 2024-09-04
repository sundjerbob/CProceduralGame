#pragma once

#include <GL/glew.h>

typedef struct {
	GLfloat* vertices;
	int vertexCount;
	GLint* indices;
	int indexCount;
	GLfloat* normals;
} Mesh;

Mesh* generatePlaneMesh(int width, int length);
Mesh* generateQuadMesh();
Mesh* updateNormals(Mesh* mesh);
Mesh* applyHeightMap(Mesh* mesh, float* heightMap);