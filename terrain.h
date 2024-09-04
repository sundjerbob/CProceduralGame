#pragma once

#include <GL/glew.h>

typedef struct {
	int** indices;
	float** weights;
	int radius;
} TerrainBrush;

float* generateHeightMap(int width, int length, float heightAmplifier, long seed, float frequency, int depth, int* offset);
float* erodeHeightMap(float* heightMap, int width, int height, TerrainBrush* brush);
TerrainBrush* createTerrainBrush(int width, int height);