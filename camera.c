#include "camera.h"

#include <stdlib.h>

#include "math2.h"

Camera* createCamera(float* targetLocation, float* targetOffset, float fov, float aspect, float near, float far)
{
    Camera* camera = (Camera*)malloc(sizeof(Camera));
    camera->position = calloc(3, sizeof(float));
    camera->rotation = calloc(3, sizeof(float));
    camera->targetLocation = targetLocation;
    camera->targetOffset = targetOffset;
    camera->fov = fov;
    camera->aspect = aspect;
    camera->near = near;
    camera->far = far;
    setPerspectiveMatrix(fov, aspect, near, far, camera->projection);
    updateCamera(camera);
    return camera;
}

void updateCamera(Camera* camera)
{
    float rotatedOffset[3];
    rotateOffset(camera->targetOffset, camera->rotation[0], camera->rotation[1], rotatedOffset);

    // Update camera position
    camera->position[0] = camera->targetLocation[0] + rotatedOffset[0];
    camera->position[1] = camera->targetLocation[1] + rotatedOffset[1];
    camera->position[2] = camera->targetLocation[2] + rotatedOffset[2];

    // Update the view matrix
    float down[] = { 0.0f, -1.0f, 0.0f };
    lookAt(camera->view, camera->position, camera->targetLocation, down);
}
