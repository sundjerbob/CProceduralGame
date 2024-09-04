#version 450 core

in vec3 position;
in vec2 texCoord;
in vec3 normal;

out vec3 Color;
out vec2 TexCoord;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec4 clipPlane;

vec3 interpolateColors(vec3 color1, vec3 color2, float factor) {
    return mix(color1, color2, factor);
}

float squishHeight(float h) {
    if (h >= 0.25 && h <= 0.7) {
        return 0.25 + pow((h - 0.25) / (0.7 - 0.25), 0.5) * (0.7 - 0.25);  // Applying sqrt() for squishing effect
    }
    return h;
}

void main()
{
    float h = position.y / 150;

    const vec4 worldLocation = model * vec4(position, 1.0);

    gl_ClipDistance[0] = dot(worldLocation, clipPlane);

    if (h < 0.2)
        Color = vec3(0.0, 0.0, 1.0);  // Deep ocean
    else if (h < 0.3)
        Color = interpolateColors(vec3(0.0, 0.0, 1.0), vec3(0.5, 0.5, 1.0), (h - 0.2) / 0.2);  // Deep to Shallow ocean
    else if (h < 0.4)
        Color = interpolateColors(vec3(0.5, 0.5, 1.0), vec3(1.0, 0.8, 0.0), (h - 0.3) / 0.1);  // Shallow ocean to Beach
    else if (h < 0.5)
        Color = interpolateColors(vec3(1.0, 0.8, 0.0), vec3(0.5, 1.0, 0.5), (h - 0.4) / 0.1);  // Beach to Plains
    else if (h < 0.7)
        Color = interpolateColors(vec3(0.5, 1.0, 0.5), vec3(0.0, 0.5, 0.0), (h - 0.5) / 0.2);  // Plains to Hill
    else if (h < 0.75)
        Color = interpolateColors(vec3(0.0, 0.5, 0.0), vec3(0.6, 0.3, 0.1), (h - 0.7) / 0.05);  // Hill to Mountain
    else if (h < 0.9)
        Color = interpolateColors(vec3(0.6, 0.3, 0.1), vec3(1.0, 1.0, 1.0), (h - 0.75) / 0.15);  // Mountain to Snow
    else
        Color = vec3(1.0, 1.0, 1.0);  // Snow

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalize(normalMatrix * normal);

    TexCoord = texCoord;

    gl_Position = projection * view * worldLocation;
}
