#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <ft2build.h>
#include <freetype/freetype.h>    

#include "util.h"
#include "math2.h"
#include "terrain.h"
#include "shader.h"
#include "mesh.h"
#include "renderer.h"
#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static float MOVE_SPEED = 2;
static float ROTATE_SPEED = 20;

#define WIDTH  1280
#define HEIGHT 720

#define CHUNK_WIDTH 512
#define CHUNK_LENGTH 512

int FPS;

float mousePosition[2];
bool mouseButtonsPressed[2];

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    mousePosition[0] = xpos;
    mousePosition[1] = HEIGHT - ypos;
}

typedef struct {
    GLuint TextureID;   // ID handle of the glyph texture
    int Size[2];        // Size of glyph
    int Bearing[2];     // Offset from baseline to left/top of glyph
    GLuint Advance;     // Horizontal offset to advance to next glyph
} Character;

Character Characters[128];
GLuint VAO, VBO;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        printf("Left mouse button pressed\n");
        mouseButtonsPressed[0] = true;
    }
    else
    {
        mouseButtonsPressed[0] = false;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) 
    {
        mouseButtonsPressed[1] = true;
        printf("Right mouse button pressed\n");
    }
    else
    {
        mouseButtonsPressed[1] = false;
    }
}

GLfloat* generateChunk(Mesh* mesh, float* offset, TerrainBrush* terrainBrush) {
    // Random seed
    long seed = rand();
    GLfloat* heightMap = generateHeightMap(CHUNK_WIDTH, CHUNK_LENGTH, 150, seed, 0.01, 10, offset);
    heightMap = erodeHeightMap(heightMap, CHUNK_WIDTH, CHUNK_LENGTH, terrainBrush);
    mesh = applyHeightMap(mesh, heightMap);
    updateNormals(mesh);
    return heightMap;
}
void RenderText(Renderer* renderer, Character* characters, GLuint vao, GLuint vbo, char* text, float x, float y, float scale)
{
    // Activate corresponding render state    
    glUseProgram(renderer->shader->program);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao);

    GLint reflectionTextureLoc = glGetUniformLocation(renderer->shader->program, "mainTexture");
    glUniform1i(reflectionTextureLoc, 0);

    // Iterate through all characters
    for (const char* c = text; *c; c++)
    {
        Character ch = Characters[(unsigned char)*c];

        float xpos = x + ch.Bearing[0] * scale;
        float ypos = y - (ch.Size[1] - ch.Bearing[1]) * scale;

        float w = ch.Size[0] * scale;
        float h = ch.Size[1] * scale;

        float xposScaled = xpos / WIDTH * 2.0f - 1.0f;
        float xposWScaled = (xpos + w) / WIDTH * 2.0f - 1.0f;

        float yposScaled = ypos / HEIGHT * 2.0f - 1.0f;
        float yposHScaled = (ypos + h) / HEIGHT * 2.0f - 1.0f;

        // Update VBO for each character
        float vertices[6][4] = {
            { xposScaled, yposHScaled, 0.0f, 0.0f },
            { xposScaled, yposScaled, 0.0f, 1.0f },
            { xposWScaled, yposScaled, 1.0f, 1.0f },

            { xposScaled,  yposHScaled, 0.0f, 0.0f },
            { xposWScaled, yposScaled, 1.0f, 1.0f },
            { xposWScaled, yposHScaled, 1.0f, 0.0f }
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


int main()
{
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // Set GLFW options
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Procedural Terrain", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Could not init FreeType Library\n");
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "fonts/Cascadia.ttf", 0, &face)) {
        fprintf(stderr, "Failed to load font\n");
        return -1;
    }


    FT_Set_Pixel_Sizes(face, 0, 48);  // Set the size to load glyphs as 48x48 pixels

    GLuint textVao;
    glGenVertexArrays(1, &textVao);
    glBindVertexArray(textVao);

    GLuint textVbo;
    glGenBuffers(1, &textVbo);
    glBindBuffer(GL_ARRAY_BUFFER, textVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


    // Load fps text to texture
    for (unsigned char c = 0; c < 128; c++) {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "Failed to load Glyph\n");
            continue;
        }
        // Generate texture
        GLuint glyphTexture;
        glGenTextures(1, &glyphTexture);
        glBindTexture(GL_TEXTURE_2D, glyphTexture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Now store character for later use
        Characters[c].TextureID = glyphTexture;
        Characters[c].Size[0] = face->glyph->bitmap.width;
        Characters[c].Size[1] = face->glyph->bitmap.rows;
        Characters[c].Bearing[0] = face->glyph->bitmap_left;
        Characters[c].Bearing[1] = face->glyph->bitmap_top;
        Characters[c].Advance = face->glyph->advance.x;
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Set random seed
    srand(getTime());

    // Generate terrain
    Shader* terrainShader = createShader("shaders/terrain.vert", "shaders/terrain.frag");

    // Allocate memory for brush indices and weights
    TerrainBrush* terrainBrush = createTerrainBrush(CHUNK_WIDTH, CHUNK_LENGTH);

    float offset[] = { 0, 0 };
    Mesh* terrainMesh = generatePlaneMesh(CHUNK_WIDTH, CHUNK_LENGTH);
    float* heightMap = generateChunk(terrainMesh, offset, terrainBrush);

    Renderer* terrainRenderer = createRenderer(terrainMesh, terrainShader, NULL, 0);

    // Load the image
    int width, height, nrChannels;
    unsigned char* data = stbi_load("images/water_du_dv.png", &width, &height, &nrChannels, 0);

    GLuint waterDuDvTexture;
    if (data) {
        glGenTextures(1, &waterDuDvTexture);
        glBindTexture(GL_TEXTURE_2D, waterDuDvTexture);

        // Set the texture wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Load the texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        // Generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }
    else {
        // Handle error
        printf("Failed to load texture\n");
        exit(1);
    }

    data = stbi_load("images/water_normals.png", &width, &height, &nrChannels, 0);
    GLuint waterNormalTexture;
    if (data) {
        glGenTextures(1, &waterNormalTexture);
        glBindTexture(GL_TEXTURE_2D, waterNormalTexture);

        // Set the texture wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Load the texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        // Generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }
    else {
        // Handle error
        printf("Failed to load texture\n");
        exit(1);
    }


    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    
    // Reflectin frame buffer
    GLuint reflectionFbo;
    glGenFramebuffers(1, &reflectionFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, reflectionFbo);

    // Create a texture to store the color buffer
    GLuint waterReflectionTexture;
    glGenTextures(1, &waterReflectionTexture);
    glBindTexture(GL_TEXTURE_2D, waterReflectionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, waterReflectionTexture, 0);


    GLuint waterReflectionDepthTexture;
    glGenTextures(1, &waterReflectionDepthTexture);
    glBindTexture(GL_TEXTURE_2D, waterReflectionDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, WIDTH, HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, waterReflectionDepthTexture, 0);


    GLuint waterReflectionDepthBuffer;
    glGenRenderbuffers(1, &waterReflectionDepthBuffer);
    glBindRenderbuffer(GL_TEXTURE_2D, waterReflectionDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, waterReflectionDepthBuffer);



    // Refraction frame buffer
    GLuint refractionFbo;
    glGenFramebuffers(1, &refractionFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, refractionFbo);

    // Create a texture to store the color buffer
    GLuint waterRefractionTexture;
    glGenTextures(1, &waterRefractionTexture);
    glBindTexture(GL_TEXTURE_2D, waterRefractionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, waterRefractionTexture, 0);


    GLuint waterRefractionDepthTexture;
    glGenTextures(1, &waterRefractionDepthTexture);
    glBindTexture(GL_TEXTURE_2D, waterRefractionDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WIDTH, HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, waterRefractionDepthTexture, 0);


    GLuint waterRefractionDepthBuffer;
    glGenRenderbuffers(1, &waterRefractionDepthBuffer);
    glBindRenderbuffer(GL_TEXTURE_2D, waterRefractionDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, waterRefractionDepthBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Water
    Shader* waterShader = createShader("shaders/water.vert", "shaders/water.frag");
    Mesh* waterMesh = generatePlaneMesh(CHUNK_WIDTH, CHUNK_LENGTH);
    GLuint waterTextures[] = { 
        waterReflectionTexture, 
        waterRefractionTexture, 
        waterDuDvTexture, 
        waterNormalTexture, 
        waterRefractionDepthTexture
    };

    Renderer* waterRenderer = createRenderer(waterMesh, waterShader, waterTextures, 5);

    // Button
    Shader* buttonShader = createShader("shaders/button.vert", "shaders/button.frag");
    Mesh* buttonMesh = generateQuadMesh();

    // Generate texture for button
    // Load the image
    data = stbi_load("images/reload.png", &width, &height, &nrChannels, 0);

    GLuint buttonTexture;
    if (data) {
        glGenTextures(1, &buttonTexture);
        glBindTexture(GL_TEXTURE_2D, buttonTexture);

        // Set the texture wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Load the texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        // Generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }
    else {
        // Handle error
        printf("Failed to load texture\n");
        exit(1);
    }

    Renderer* buttonRenderer = createRenderer(buttonMesh, buttonShader, &buttonTexture, 1);

    Mesh* textMesh = generateQuadMesh();
    Shader* textShader = createShader("shaders/text.vert", "shaders/text.frag");
    Renderer* textRenderer = createRenderer(textMesh, textShader, 0, 0);

    // Set render mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    long lastFrameTime = getTime();
    float deltaTime = 0.0f;
    int framesCount = 0;
    float timePassed = 0.0f;

    float terrainTranslation[] = { 0.0f, -0.55f, 0.0f };
    float terrainRotation[] = { 0.0f, 0.0f, 0.0f };
    float terrainScale[] = { 0.01f, 0.01f, 0.01f };

    float waterTranslation[] = { 0.0f, 0.0f, 0.0f };
    float waterRotation[] = { 0.0f, 0.0f, 0.0f };
    float waterScale[] = { 0.01f, 0.01f, 0.01f };

    float cameraTarget[] = { 256.0f * 0.01f, 0.0f, 256.0f * 0.01f };
    float cameraOffset[] = { 0.0f, 2.0f, -2.0f };

    float buttonPosition[] = { 0.85f, 0.81f };
    float buttonScale[] = { 0.08f, 0.08f};

    buttonScale[1] *= (float)WIDTH / HEIGHT;



    Camera* camera = createCamera(cameraTarget, cameraOffset, 30.0, (float) WIDTH / HEIGHT, 0.01f, 1000.0f);

    glEnable(GL_BLEND);

    GLFWcursor* defaultCursor = glfwCreateStandardCursor(GLFW_CURSOR_NORMAL);
    GLFWcursor* handCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

    // Set default cursor
    glfwSetCursor(window, defaultCursor);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        float currentTime = getTime();
        float deltaTime = (float) ((currentTime - lastFrameTime) / 1000);
        lastFrameTime = currentTime;

        // Calculate FPS
        ++framesCount;
        timePassed += deltaTime;
        if (timePassed >= 1.0)
        {
            FPS = (int) (framesCount / timePassed);
            framesCount = 0;
            timePassed = 0.0f;
        }

        // Process input
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera->rotation[1] -= ROTATE_SPEED * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera->rotation[1] += ROTATE_SPEED * deltaTime;

        float radians = toRadians(camera->rotation[1]);

        float forwardX = cosf(radians);
        float forwardZ = sinf(radians);

        // Strafing Left/Right (A/D keys)
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera->targetLocation[0] += forwardX * MOVE_SPEED * deltaTime;
            camera->targetLocation[2] -= forwardZ * MOVE_SPEED * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera->targetLocation[0] -= forwardX * MOVE_SPEED * deltaTime;
            camera->targetLocation[2] += forwardZ * MOVE_SPEED * deltaTime;
        }

        // Moving Forward/Backward (W/S keys)
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera->targetLocation[0] += forwardZ * MOVE_SPEED * deltaTime;
            camera->targetLocation[2] += forwardX * MOVE_SPEED * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera->targetLocation[0] -= forwardZ * MOVE_SPEED * deltaTime;
            camera->targetLocation[2] -= forwardX * MOVE_SPEED * deltaTime;
        }

        // Moving Up/Down (Space/Shift keys)
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            camera->targetOffset[1] += MOVE_SPEED * deltaTime;
            camera->targetOffset[2] -= MOVE_SPEED * 0.5f * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            camera->targetOffset[1] -= MOVE_SPEED * deltaTime;
            camera->targetOffset[2] += MOVE_SPEED * 0.5f * deltaTime;
        }

        // Update camera
        updateCamera(camera);

        //printf("MPOS: %.2f %.2f\n", mousePosition[0], mousePosition[1]);

        float buttonPixelPosition[] = {
            (buttonPosition[0] + 1) / 2.0f * WIDTH,
            (buttonPosition[1] + 1) / 2.0f * HEIGHT
        };

        float buttonPixelScale[] = {
            buttonScale[0] * WIDTH,
            buttonScale[1] * HEIGHT
        };

        // Is mouse hovering button
        if (mousePosition[0] >= buttonPixelPosition[0] && mousePosition[0] <= buttonPixelPosition[0] + buttonPixelScale[0] &&
            mousePosition[1] >= buttonPixelPosition[1] && mousePosition[1] <= buttonPixelPosition[1] + buttonPixelScale[1])
        {
            glfwSetCursor(window, handCursor);
            if (mouseButtonsPressed[0])
            {
                printf("New chunk generating...\n");
                heightMap = generateChunk(terrainMesh, offset, terrainBrush);
                printf("New chunk generated!\n");
            }
        }
        else
        {
            glfwSetCursor(window, defaultCursor);
        }

        glEnable(GL_CLIP_DISTANCE0);

        // Update objects
        float terrainModelMatrix[16];
        setModelMatrix(terrainTranslation, terrainRotation, terrainScale, terrainModelMatrix);

        // Update objects
        float waterModelMatrix[16];
        setModelMatrix(waterTranslation, waterRotation, waterScale, waterModelMatrix);


        glEnable(GL_DEPTH_TEST);

        // Clear the screen to background color
        glClearColor(0.0f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render objects
         
        // Enable depth testing
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


        // REFLECTION
        glBindFramebuffer(GL_FRAMEBUFFER, reflectionFbo);

        glClearColor(0.0f, 0.7f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float reflectionClipPlane[] = { 0.0f, 1.0f, 0.0f, -waterTranslation[1] };

        camera->targetOffset[1] *= -1;
        updateCamera(camera);
        renderMesh(terrainRenderer, terrainModelMatrix, camera, reflectionClipPlane);
        camera->targetOffset[1] *= -1;
        updateCamera(camera);

        // REFRACTION
        glBindFramebuffer(GL_FRAMEBUFFER, refractionFbo);

        float refractionClipPlane[] = { 0.0f, -1.0f, 0.0f, waterTranslation[1] };

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderMesh(terrainRenderer, terrainModelMatrix, camera, refractionClipPlane);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDisable(GL_CLIP_DISTANCE0);

        renderMesh(terrainRenderer, terrainModelMatrix, camera, NULL);
        renderMesh(waterRenderer, waterModelMatrix, camera, NULL);

        glDisable(GL_DEPTH_TEST);
        renderUI(buttonRenderer, buttonPosition, buttonScale);
        float textColor[] = { 1.0f, 1.0f, 1.0f };
        char fpsString[16];
        sprintf_s(fpsString, 16, "FPS:%d", FPS);
        RenderText(textRenderer, Characters, textVao, textVbo, fpsString, 10.0f, 660.0f, 1.0f);
        RenderText(textRenderer, Characters, textVao, textVbo, "REGENERATE", 1170.0f, 630.0f, 0.3f);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Clean up
    cleanShader(terrainShader);
    cleanRenderer(terrainRenderer);

    // Terminate GLFW
    glfwTerminate();

    return 0;
}
