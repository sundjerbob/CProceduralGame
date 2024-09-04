#version 450 core

in vec3 Color;  // The color passed from the vertex shader
in vec3 Normal; // The normal passed from the vertex shader
in vec2 TexCoord;

out vec4 FragColor; // The final color output

uniform sampler2D texture1;

// Constant light direction, color, and intensity
uniform vec3 lightDir = normalize(vec3(0.5, -1.0, 0.5)); // Light coming from top-right
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0); // White light

void main()
{
    // Normalize the normal vector
    vec3 norm = normalize(Normal);

    // Calculate the diffuse component using Lambert's cosine law
    float diff = max(dot(norm, -lightDir), 0.65);

    // Apply the diffuse light to the base color with intensity
    vec3 diffuse = diff * lightColor * Color;

    // Set the output color
    FragColor = vec4(diffuse, 1.0);
    //FragColor = texture(texture1, TexCoord);
}
