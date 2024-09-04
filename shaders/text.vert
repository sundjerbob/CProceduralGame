#version 450 core

in vec4 vertex;

out vec2 TexCoord;

void main()
{
    TexCoord = vec2(vertex.z, vertex.w);
    gl_Position = vec4(vertex.xy, 0.0, 1.0);
}
