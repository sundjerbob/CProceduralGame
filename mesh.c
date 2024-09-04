#include "mesh.h"

#include <stdlib.h>
#include <stdio.h>

#include "math2.h"

Mesh* generatePlaneMesh(int width, int length)
{
    Mesh* mesh = (Mesh*) malloc(sizeof(Mesh));

    mesh->vertexCount = width * length;
    mesh->vertices = (GLfloat*) malloc(mesh->vertexCount * 5 * sizeof(GLfloat));

    mesh->normals = (GLfloat*)malloc(mesh->vertexCount * 3 * sizeof(GLfloat));

    mesh->indexCount = (width - 1) * (length - 1) * 6;
    mesh->indices = (GLint*) malloc(mesh->indexCount * sizeof(GLint));


    int vertexIndex = 0;
    int index = 0;

    for (int z = 0; z < length; z++)
    {
        for (int x = 0; x < width; x++)
        {
            // Position coords
            mesh->vertices[vertexIndex++] = x;
            mesh->vertices[vertexIndex++] = 0.0f;
            mesh->vertices[vertexIndex++] = z;

            // Texture coords
            mesh->vertices[vertexIndex++] = (float)x / (float)(width - 1);
            mesh->vertices[vertexIndex++] = (float)z / (float)(length - 1);

            // Calculate indices for two triangles, but only if not on the last row/column
            if (z < length - 1 && x < width - 1)
            {
                int topLeft = z * width + x;
                int topRight = topLeft + 1;
                int bottomLeft = (z + 1) * width + x;
                int bottomRight = bottomLeft + 1;

                // First triangle
                mesh->indices[index++] = topLeft;
                mesh->indices[index++] = bottomLeft;
                mesh->indices[index++] = topRight;

                // Second triangle
                mesh->indices[index++] = topRight;
                mesh->indices[index++] = bottomLeft;
                mesh->indices[index++] = bottomRight;
            }
        }
    }

    mesh = updateNormals(mesh);

    return mesh;
}

Mesh* generateQuadMesh()
{
    Mesh* mesh = (Mesh*)malloc(sizeof(Mesh));

    int width = 2;
    int height = 2;

    mesh->vertexCount = width * height;
    mesh->vertices = (GLfloat*)malloc(mesh->vertexCount * 5 * sizeof(GLfloat));

    mesh->normals = (GLfloat*)malloc(mesh->vertexCount * 3 * sizeof(GLfloat));

    mesh->indexCount = (width - 1) * (height - 1) * 6;
    mesh->indices = (GLint*)malloc(mesh->indexCount * sizeof(GLint));


    int vertexIndex = 0;
    int index = 0;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Position coords
            mesh->vertices[vertexIndex++] = x;
            mesh->vertices[vertexIndex++] = y;
            mesh->vertices[vertexIndex++] = 0.0f;

            // Texture coords
            mesh->vertices[vertexIndex++] = (float)x / (float)(width - 1);
            mesh->vertices[vertexIndex++] = (float)y / (float)(height - 1);

            // Calculate indices for two triangles, but only if not on the last row/column
            if (y < height - 1 && x < width - 1)
            {
                int topLeft = y * width + x;
                int topRight = topLeft + 1;
                int bottomLeft = (y + 1) * width + x;
                int bottomRight = bottomLeft + 1;

                // First triangle
                mesh->indices[index++] = topLeft;
                mesh->indices[index++] = bottomLeft;
                mesh->indices[index++] = topRight;

                // Second triangle
                mesh->indices[index++] = topRight;
                mesh->indices[index++] = bottomLeft;
                mesh->indices[index++] = bottomRight;
            }
        }
    }

    mesh = updateNormals(mesh);

    return mesh;
}

Mesh* updateNormals(Mesh* mesh)
{
    // Initialize all normals to zero
    memset(mesh->normals, 0, mesh->vertexCount * 3 * sizeof(GLfloat));

    // Accumulate face normals for each vertex
    const faceCount = mesh->indexCount / 3;
    for (int i = 0; i < faceCount; i++) {
        int indexA = mesh->indices[i * 3];
        int indexB = mesh->indices[i * 3 + 1];
        int indexC = mesh->indices[i * 3 + 2];

        float vecA[3] = { mesh->vertices[indexA * 5], mesh->vertices[indexA * 5 + 1], mesh->vertices[indexA * 5 + 2] };
        float vecB[3] = { mesh->vertices[indexB * 5], mesh->vertices[indexB * 5 + 1], mesh->vertices[indexB * 5 + 2] };
        float vecC[3] = { mesh->vertices[indexC * 5], mesh->vertices[indexC * 5 + 1], mesh->vertices[indexC * 5 + 2] };

        float edge1[3] = { vecB[0] - vecA[0], vecB[1] - vecA[1], vecB[2] - vecA[2] };
        float edge2[3] = { vecC[0] - vecA[0], vecC[1] - vecA[1], vecC[2] - vecA[2] };

        float faceNormal[3];
        crossProduct(faceNormal, edge1, edge2);
        normalize(faceNormal);

        mesh->normals[indexA * 3] += faceNormal[0];
        mesh->normals[indexA * 3 + 1] += faceNormal[1];
        mesh->normals[indexA * 3 + 2] += faceNormal[2];

        mesh->normals[indexB * 3] += faceNormal[0];
        mesh->normals[indexB * 3 + 1] += faceNormal[1];
        mesh->normals[indexB * 3 + 2] += faceNormal[2];

        mesh->normals[indexC * 3] += faceNormal[0];
        mesh->normals[indexC * 3 + 1] += faceNormal[1];
        mesh->normals[indexC * 3 + 2] += faceNormal[2];
    }

    // Normalize the accumulated normals
    for (int i = 0; i < mesh->vertexCount; i++) {
        normalize(&mesh->normals[i * 3]);
    }

    return mesh;
}

Mesh* applyHeightMap(Mesh* mesh, float* heightMap)
{
    for (int i = 0; i < mesh->vertexCount; i++)
        mesh->vertices[i * 5 + 1] = heightMap[i];

    updateNormals(mesh);

    return mesh;
}

Mesh* translateMesh(Mesh* mesh, float* offset)
{
    for (int i = 0; i < mesh->vertexCount; i++) {
        mesh->vertices[i * 5] = offset[0];
        mesh->vertices[i * 5 + 1] = offset[1];
        mesh->vertices[i * 5 + 2] = offset[2];
    }

    updateNormals(mesh);

    return mesh;
}