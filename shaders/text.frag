#version 450 core

in vec2 TexCoord;

out vec4 FragColor; // The final color output

uniform sampler2D mainTexture;

void main()
{
    // Set the output color
    FragColor = vec4(1.0, 1.0, 1.0, texture(mainTexture, TexCoord).r);
}
