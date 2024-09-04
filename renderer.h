#pragma once

#include <GL/glew.h>
#include "mesh.h"
#include "shader.h"
#include "camera.h"

typedef struct {
    GLuint vao;
    GLuint vbo[2]; // 0 - Vertices Array, 1 - Normals Array
    GLuint ebo;
    Mesh* mesh;
    Shader* shader;
    GLint* textures;
    int texturesCount;
} Renderer;

Renderer* createRenderer(Mesh* mesh, Shader* shader, GLuint* textures, int texturesCount);
void renderMesh(Renderer* renderer, float* model, Camera* camera, float* clipPlane);
void renderUI(Renderer* renderer, float* offset, float* scale);
void cleanRenderer(Renderer* renderer);