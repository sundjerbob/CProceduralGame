#include "renderer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "math2.h"
#include "mesh.h";
#include "shader.h";


Renderer* createRenderer(Mesh* mesh, Shader* shader, GLuint* textures, int texturesCount)
{
    Renderer* renderer = (Renderer*)malloc(sizeof(Renderer));
    glGenVertexArrays(1, &renderer->vao);
    glGenBuffers(1, &renderer->vbo[0]); // Vertices
    glGenBuffers(1, &renderer->vbo[1]); // Normals
    glGenBuffers(1, &renderer->ebo);
    renderer->mesh = mesh;
    renderer->shader = shader;
    renderer->textures = textures;
    renderer->texturesCount = texturesCount;
    return renderer;
}

void renderMesh(Renderer* renderer, float* model, Camera* camera, float* clipPlane)
{
    // Generate and bind a VAO for each terrain chunk
    glBindVertexArray(renderer->vao);

    // Create a Vertex Buffer Object and copy the vertex data to it
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, renderer->mesh->vertexCount * 5 * sizeof(GLfloat), renderer->mesh->vertices, GL_STATIC_DRAW);

    GLint positionAttribute = glGetAttribLocation(renderer->shader->program, "position");
    glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(positionAttribute);

    GLint textureAttribute = glGetAttribLocation(renderer->shader->program, "texCoord");
    glVertexAttribPointer(textureAttribute, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(textureAttribute);

    // Create a Normals Buffer Object and copy the vertex data to it
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, renderer->mesh->vertexCount * 3 * sizeof(GLfloat), renderer->mesh->normals, GL_STATIC_DRAW);

    GLint normalsAttribute = glGetAttribLocation(renderer->shader->program, "normal");
    glVertexAttribPointer(normalsAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(normalsAttribute);

    // Create an Element Buffer Object and copy the index data to it
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer->mesh->indexCount * sizeof(GLuint), renderer->mesh->indices, GL_STATIC_DRAW);

    // Render
    glUseProgram(renderer->shader->program);

    if (renderer->texturesCount > 0) {
        glActiveTexture(GL_TEXTURE0);  // Activate the texture unit
        glBindTexture(GL_TEXTURE_2D, renderer->textures[0]);  // Bind the texture to GL_TEXTURE_2D target
    }

    if (renderer->texturesCount > 1) {
        glActiveTexture(GL_TEXTURE1);  // Activate the texture unit
        glBindTexture(GL_TEXTURE_2D, renderer->textures[1]);  // Bind the texture to GL_TEXTURE_2D target
    }

    if (renderer->texturesCount > 2) {
        glActiveTexture(GL_TEXTURE2);  // Activate the texture unit
        glBindTexture(GL_TEXTURE_2D, renderer->textures[2]);  // Bind the texture to GL_TEXTURE_2D target
    }

    if (renderer->texturesCount > 3) {
        glActiveTexture(GL_TEXTURE3);  // Activate the texture unit
        glBindTexture(GL_TEXTURE_2D, renderer->textures[3]);  // Bind the texture to GL_TEXTURE_2D target
    }

    if (renderer->texturesCount > 4) {
        glActiveTexture(GL_TEXTURE4);  // Activate the texture unit
        glBindTexture(GL_TEXTURE_2D, renderer->textures[4]);  // Bind the texture to GL_TEXTURE_2D target
    }

    GLint viewLoc = glGetUniformLocation(renderer->shader->program, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera->view);

    GLint projLoc = glGetUniformLocation(renderer->shader->program, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, camera->projection);

    glBindVertexArray(renderer->vao);

    GLint modelLoc = glGetUniformLocation(renderer->shader->program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);

    viewLoc = glGetUniformLocation(renderer->shader->program, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, camera->view);

    projLoc = glGetUniformLocation(renderer->shader->program, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, camera->projection);

    GLint cameraPositionLoc = glGetUniformLocation(renderer->shader->program, "cameraPosition");
    glUniform3fv(cameraPositionLoc, 1, camera->position); 

    GLint clipPlaneLoc = glGetUniformLocation(renderer->shader->program, "clipPlane");
    if (clipPlaneLoc != -1 && clipPlane != NULL)
        glUniform4fv(clipPlaneLoc, 1, clipPlane);

    GLint reflectionTextureLoc = glGetUniformLocation(renderer->shader->program, "reflectionTexture");
    glUniform1i(reflectionTextureLoc, 0);

    GLint refractionTextureLoc = glGetUniformLocation(renderer->shader->program, "refractionTexture");
    glUniform1i(refractionTextureLoc, 1);

    GLint duDvTextureLoc = glGetUniformLocation(renderer->shader->program, "duDvTexture");
    glUniform1i(duDvTextureLoc, 2);

    GLint normalTextureLoc = glGetUniformLocation(renderer->shader->program, "normalMap");
    glUniform1i(normalTextureLoc, 3);

    GLint depthTextureLoc = glGetUniformLocation(renderer->shader->program, "depthMap");
    glUniform1i(depthTextureLoc, 4);

    GLint timeLoc = glGetUniformLocation(renderer->shader->program, "time");
    glUniform1f(timeLoc, glfwGetTime());

    glDrawElements(GL_TRIANGLES, renderer->mesh->indexCount, GL_UNSIGNED_INT, 0);
}

void renderUI(Renderer* renderer, float* offset, float* scale)
{
    // Generate and bind a VAO for each terrain chunk
    glBindVertexArray(renderer->vao);

    // Create a Vertex Buffer Object and copy the vertex data to it
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, renderer->mesh->vertexCount * 5 * sizeof(GLfloat), renderer->mesh->vertices, GL_STATIC_DRAW);

    GLint positionAttribute = glGetAttribLocation(renderer->shader->program, "position");
    glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(positionAttribute);

    GLint textureAttribute = glGetAttribLocation(renderer->shader->program, "texCoord");
    glVertexAttribPointer(textureAttribute, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(textureAttribute);

    // Create an Element Buffer Object and copy the index data to it
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, renderer->mesh->indexCount * sizeof(GLuint), renderer->mesh->indices, GL_STATIC_DRAW);

    // Render
    glUseProgram(renderer->shader->program);

    if (renderer->texturesCount > 0) {
        glActiveTexture(GL_TEXTURE0);  // Activate the texture unit
        glBindTexture(GL_TEXTURE_2D, renderer->textures[0]);  // Bind the texture to GL_TEXTURE_2D target
        GLint mainTextureLoc = glGetUniformLocation(renderer->shader->program, "mainTexture");
        glUniform1i(mainTextureLoc, GL_TEXTURE0);
    }

    GLint offsetLoc = glGetUniformLocation(renderer->shader->program, "offset");
    glUniform2fv(offsetLoc, 1, offset);

    GLint scaleLoc = glGetUniformLocation(renderer->shader->program, "scale");
    glUniform2fv(scaleLoc, 1, scale);

    glDrawElements(GL_TRIANGLES, renderer->mesh->indexCount, GL_UNSIGNED_INT, 0);
}

void cleanRenderer(Renderer* renderer)
{
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteBuffers(1, &renderer->ebo);
    glDeleteVertexArrays(1, &renderer->vao);
}