#version 450 core

in vec3 position;
in vec2 texCoord;

uniform vec2 offset;
uniform vec2 scale;

out vec2 TexCoord;

void main()
{
    TexCoord = texCoord;
    vec2 pos = position.xy * scale + offset;
    gl_Position = vec4(pos, 0.0, 1.0);
}
