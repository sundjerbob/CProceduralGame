#include "math2.h"

#include <math.h>

float toRadians(float degrees)
{
    return degrees * (PI / 180.0f);
}

void normalize(float* v) {
    float length = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (length != 0.0f) {
        v[0] /= length;
        v[1] /= length;
        v[2] /= length;
    }
}

void crossProduct(float* result, const float* a, const float* b) {
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
}

float dotProduct(const float* a, const float* b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void setModelMatrix(float translation[3], float rotation[3], float scale[3], float* matrix) {
    // Translation matrix
    float tx = translation[0];
    float ty = translation[1];
    float tz = translation[2];

    // Rotation angles (in radians)
    float xRadians = rotation[0] * (PI / 180.0f);
    float yRadians = rotation[1] * (PI / 180.0f);
    float zRadians = rotation[2] * (PI / 180.0f);

    // Scaling factors
    float sx = scale[0];
    float sy = scale[1];
    float sz = scale[2];

    // Compute rotation matrices
    float cosX = cosf(xRadians), sinX = sinf(xRadians);
    float cosY = cosf(yRadians), sinY = sinf(yRadians);
    float cosZ = cosf(zRadians), sinZ = sinf(zRadians);

    // Initialize identity matrix
    matrix[0] = sx * cosY * cosZ;
    matrix[1] = sy * (cosZ * sinX * sinY + cosX * sinZ);
    matrix[2] = sz * (sinX * sinZ - cosX * cosZ * sinY);
    matrix[3] = 0.0f;

    matrix[4] = -sy * cosY * sinZ;
    matrix[5] = sx * (cosX * cosZ - sinX * sinY * sinZ);
    matrix[6] = sz * (cosX * sinY * sinZ + cosZ * sinX);
    matrix[7] = 0.0f;

    matrix[8] = sx * sinY;
    matrix[9] = sy * -cosY * sinX;
    matrix[10] = sz * cosX * cosY;
    matrix[11] = 0.0f;

    matrix[12] = tx;
    matrix[13] = ty;
    matrix[14] = tz;
    matrix[15] = 1.0f;
}

void lookAt(float* viewMatrix, const float* eye, const float* center, const float* down) {
    float forward[3], right[3], upVector[3];

    // Calculate forward vector (direction from eye to center)
    forward[0] = center[0] - eye[0];
    forward[1] = center[1] - eye[1];
    forward[2] = center[2] - eye[2];
    normalize(forward);

    // Invert the forward vector to align with the camera's viewing direction
    forward[0] = -forward[0];
    forward[1] = -forward[1];
    forward[2] = -forward[2];

    // Calculate right vector (cross product of up and forward vectors)
    crossProduct(right, down, forward);
    normalize(right);

    // Calculate the corrected up vector (cross product of forward and right vectors)
    crossProduct(upVector, forward, right);

    // Set up the view matrix
    viewMatrix[0] = right[0];
    viewMatrix[1] = upVector[0];
    viewMatrix[2] = forward[0];
    viewMatrix[3] = 0.0f;

    viewMatrix[4] = right[1];
    viewMatrix[5] = upVector[1];
    viewMatrix[6] = forward[1];
    viewMatrix[7] = 0.0f;

    viewMatrix[8] = right[2];
    viewMatrix[9] = upVector[2];
    viewMatrix[10] = forward[2];
    viewMatrix[11] = 0.0f;

    viewMatrix[12] = -dotProduct(right, eye);
    viewMatrix[13] = -dotProduct(upVector, eye);
    viewMatrix[14] = -dotProduct(forward, eye);
    viewMatrix[15] = 1.0f;
}

void updateViewMatrix(float* viewMatrix, float eye[3], float forward[3], float up[3]) {
    float center[] = { eye[0] + forward[0], eye[1] + forward[1], eye[2] + forward[2] };
    lookAt(viewMatrix, eye, center, up);
}

void setPerspectiveMatrix(float fov, float aspect, float near, float far, float* matrix) {
    float tanHalfFov = tanf(fov / 2.0f);
    float range = near - far;

    matrix[0] = 1.0f / (aspect * tanHalfFov);
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = 1.0f / tanHalfFov;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = (near + far) / range;
    matrix[11] = -1.0f;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = (2.0f * near * far) / range;
    matrix[15] = 0.0f;
}

void rotateOffset(float* offset, float xAngle, float yAngle, float* resultOffset) {
    // Extract initial offset values
    float offsetX = offset[0];
    float offsetY = offset[1];
    float offsetZ = offset[2];

    // Rotation around the X-axis (pitch)
    float pitchRadians = xAngle * (PI / 180.0f);
    float cosPitch = cosf(pitchRadians);
    float sinPitch = sinf(pitchRadians);

    float rotatedY = offsetY * cosPitch - offsetZ * sinPitch;
    float rotatedZ = offsetY * sinPitch + offsetZ * cosPitch;

    // Rotation around the Y-axis (yaw)
    float yawRadians = yAngle * (PI / 180.0f);
    float cosYaw = cosf(yawRadians);
    float sinYaw = sinf(yawRadians);

    resultOffset[0] = offsetX * cosYaw + rotatedZ * sinYaw;
    resultOffset[1] = rotatedY;
    resultOffset[2] = -offsetX * sinYaw + rotatedZ * cosYaw;
}