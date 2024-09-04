
#include "terrain.h"
#include <stdlib.h>
#include <stdio.h>
#include "noise.h"
#include <time.h>
#include <math.h>

float* generateHeightMap(int width, int length, float heightAmplifier, long seed, float frequency, int depth, int* offset)
{
    int vertexCount = width * length;
    float* heightMap = (GLfloat*) calloc(vertexCount, sizeof(GLfloat));
    if (heightMap == NULL)
        return NULL;

    for (int z = 0; z < length; z++)
    {
        for (int x = 0; x < width; x++)
        {
            heightMap[z * width + x] = perlin2d(x + offset[0], z + offset[1], seed, frequency, depth) * heightAmplifier;
        }
    }

    return heightMap;
}

// Erosion Parameters
#define EROSION_RADIUS 3
#define INERTIA 0.1f
#define SEDIMENT_CAPACITY_FACTOR 4.0f
#define MIN_SEDIMENT_CAPACITY 0.01f
#define ERODE_SPEED 0.1f
#define DEPOSIT_SPEED 0.01f
#define EVAPORATE_SPEED 0.05f
#define GRAVITY 4.0f
#define MAX_DROPLET_LIFETIME 100

// Helper struct for holding height and gradient data
typedef struct {
    float height;
    float gradientX;
    float gradientY;
} HeightAndGradient;

// Function Prototypes
HeightAndGradient calculateHeightAndGradient(float* heightMap, int width, int height, float posX, float posY);

// Erosion function
float* erodeHeightMap(float* heightMap, int width, int height, TerrainBrush* brush) {
    // Erosion loop
    for (int iteration = 0; iteration < 100000; iteration++) {
        float posX = (float)(rand() % width);
        float posY = (float)(rand() % height);
        float dirX = 0;
        float dirY = 0;
        float speed = 1.0f;
        float water = 1.0f;
        float sediment = 0.0f;

        for (int lifetime = 0; lifetime < MAX_DROPLET_LIFETIME; lifetime++) {
            int nodeX = (int)posX;
            int nodeY = (int)posY;
            int dropletIndex = nodeY * width + nodeX;

            if (dropletIndex < 0 || dropletIndex >= width * height) {
                printf("Error: dropletIndex out of bounds (%d)\n", dropletIndex);
                break;
            }

            HeightAndGradient heightAndGradient = calculateHeightAndGradient(heightMap, width, height, posX, posY);

            // Update the droplet's direction and position
            dirX = dirX * INERTIA - heightAndGradient.gradientX * (1 - INERTIA);
            dirY = dirY * INERTIA - heightAndGradient.gradientY * (1 - INERTIA);
            float len = sqrtf(dirX * dirX + dirY * dirY);
            if (len != 0) {
                dirX /= len;
                dirY /= len;
            }
            posX += dirX;
            posY += dirY;

            if (posX < 0 || posX >= width - 1 || posY < 0 || posY >= height - 1) {
                break;
            }

            // Find the droplet's new height and calculate the deltaHeight
            float newHeight = calculateHeightAndGradient(heightMap, width, height, posX, posY).height;
            float deltaHeight = newHeight - heightAndGradient.height;

            // Calculate sediment capacity
            float sedimentCapacity = fmaxf(-deltaHeight * speed * water * SEDIMENT_CAPACITY_FACTOR, MIN_SEDIMENT_CAPACITY);

            if (sediment > sedimentCapacity || deltaHeight > 0) {
                float amountToDeposit = (deltaHeight > 0) ? fminf(deltaHeight, sediment) : (sediment - sedimentCapacity) * DEPOSIT_SPEED;
                sediment -= amountToDeposit;

                // Add the sediment to the four nodes of the current cell using bilinear interpolation
                float cellOffsetX = posX - nodeX;
                float cellOffsetY = posY - nodeY;
                heightMap[dropletIndex] += amountToDeposit * (1 - cellOffsetX) * (1 - cellOffsetY);
                heightMap[dropletIndex + 1] += amountToDeposit * cellOffsetX * (1 - cellOffsetY);
                heightMap[dropletIndex + width] += amountToDeposit * (1 - cellOffsetX) * cellOffsetY;
                heightMap[dropletIndex + width + 1] += amountToDeposit * cellOffsetX * cellOffsetY;
            } else {
                // Erode the terrain
                float amountToErode = fminf((sedimentCapacity - sediment) * ERODE_SPEED, -deltaHeight);
                int maxPoints = (2 * EROSION_RADIUS + 1) * (2 * EROSION_RADIUS + 1);

                for (int brushPointIndex = 0; brushPointIndex < maxPoints; brushPointIndex++) {
                    int nodeIndex = brush->indices[dropletIndex][brushPointIndex];
                    float weighedErodeAmount = amountToErode * brush->weights[dropletIndex][brushPointIndex];
                    float deltaSediment = fminf(heightMap[nodeIndex], weighedErodeAmount);
                    heightMap[nodeIndex] -= deltaSediment;
                    heightMap[nodeIndex] = fmaxf(heightMap[nodeIndex], 0);
                    sediment += deltaSediment;
                }
            }

            // Update droplet's speed and water content
            speed = sqrtf(speed * speed + deltaHeight * GRAVITY);
            water *= (1 - EVAPORATE_SPEED);
        }
    }

    return heightMap;
}

HeightAndGradient calculateHeightAndGradient(float* heightMap, int width, int height, float posX, float posY) {
    int coordX = (int)posX;
    int coordY = (int)posY;

    // Calculate droplet's offset inside the cell
    float x = posX - coordX;
    float y = posY - coordY;

    // Calculate heights of the four nodes of the droplet's cell
    int nodeIndexNW = coordY * width + coordX;
    float heightNW = heightMap[nodeIndexNW];
    float heightNE = heightMap[nodeIndexNW + 1];
    float heightSW = heightMap[nodeIndexNW + width];
    float heightSE = heightMap[nodeIndexNW + width + 1];

    // Calculate droplet's direction of flow with bilinear interpolation
    float gradientX = (heightNE - heightNW) * (1 - y) + (heightSE - heightSW) * y;
    float gradientY = (heightSW - heightNW) * (1 - x) + (heightSE - heightNE) * x;

    // Calculate height with bilinear interpolation
    float newHeight = heightNW * (1 - x) * (1 - y) + heightNE * x * (1 - y) + heightSW * (1 - x) * y + heightSE * x * y;

    HeightAndGradient result;
    result.height = newHeight;
    result.gradientX = gradientX;
    result.gradientY = gradientY;

    return result;
}

TerrainBrush* createTerrainBrush(int width, int height) {
    TerrainBrush* brush = (TerrainBrush*)malloc(sizeof(TerrainBrush));

    brush->indices = (int**)malloc(width * height * sizeof(int*));
    brush->weights = (float**)malloc(width * height * sizeof(float*));
    brush->radius = EROSION_RADIUS;

    const int maxPoints = (2 * brush->radius + 1) * (2 * brush->radius + 1);

    for (int i = 0; i < width * height; i++) {
        brush->indices[i] = (int*)calloc(maxPoints, sizeof(int));
        brush->weights[i] = (float*)calloc(maxPoints, sizeof(float));

        // Initialize the brush indices and weights
        int centreX = i % width;
        int centreY = i / width;
        int addIndex = 0;

        for (int y = -brush->radius; y <= brush->radius; y++) {
            for (int x = -brush->radius; x <= brush->radius; x++) {
                float sqrDst = x * x + y * y;
                if (sqrDst < brush->radius * brush->radius) {
                    int coordX = centreX + x;
                    int coordY = centreY + y;

                    if (coordX >= 0 && coordX < width && coordY >= 0 && coordY < height) {
                        float weight = 1 - sqrtf(sqrDst) / brush->radius;
                        brush->indices[i][addIndex] = coordY * width + coordX;
                        brush->weights[i][addIndex] = weight;
                    }
                }

                addIndex++;
            }
        }
    }
    
    return brush;
}
