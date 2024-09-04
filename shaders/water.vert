#version 330 core

in vec3 position;
in vec2 texCoord;

out vec4 clipSpace;
out vec2 TexCoord;
out vec3 toCameraVector;
out vec3 fromLightVector;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPosition;

uniform float time;

vec3 lightDir = normalize(vec3(-1.0, -1.0, -1.0));

const float tiling = 8.0;

const float waveAmplitude = 0.8; // Amplitude of the sine wave
const float waveFrequency = 0.2; // Frequency of the sine wave

void main() 
{
    float wave = sin(position.x * waveFrequency + time) * waveAmplitude;

    vec4 worldPosition = model * vec4(position.x, 0.0, position.z, 1.0);
    clipSpace = projection * view * worldPosition;
    gl_Position = clipSpace;

    TexCoord = texCoord * tiling;
    toCameraVector = cameraPosition - worldPosition.xyz;
    fromLightVector = worldPosition.xyz - lightDir;
}
