#include "shader.h"

#include "stdlib.h"
#include "stdio.h"

#include "util.h"

Shader* createShader(char* vertexShaderPath, char* fragmentShaderPath)
{
    Shader* shader = (Shader*) malloc(sizeof(Shader));
    if (shader == NULL)
        return NULL;

    // Create and compile the vertex shader
    shader->vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vertexSource = readFileToString(vertexShaderPath);
    glShaderSource(shader->vertexShader, 1, &vertexSource, NULL);
    glCompileShader(shader->vertexShader);

    // Check for compile errors
    GLint status;
    glGetShaderiv(shader->vertexShader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(shader->vertexShader, 512, NULL, buffer);
        fprintf(stderr, "Vertex Shader Compile Error: %s\n", buffer);
    }

    // Create and compile the fragment shader
    shader->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fragmentSource = readFileToString(fragmentShaderPath);
    glShaderSource(shader->fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(shader->fragmentShader);

    // Check for compile errors
    glGetShaderiv(shader->fragmentShader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(shader->fragmentShader, 512, NULL, buffer);
        fprintf(stderr, "Fragment Shader Compile Error: %s\n", buffer);
    }

    // Link the vertex and fragment shader into a shader program
    shader->program = glCreateProgram();
    glAttachShader(shader->program, shader->vertexShader);
    glAttachShader(shader->program, shader->fragmentShader);
    glLinkProgram(shader->program);

    return shader;
}

void cleanShader(Shader* shader)
{
    glDeleteShader(shader->vertexShader);
    glDeleteShader(shader->fragmentShader);
    glDeleteProgram(shader->program);
    free(shader);
}
