#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 eyePosition;
out vec3 eyeNormal;
out vec3 worldPosition;
out vec2 patchUV;
flat out vec3 flatNormal;
flat out vec3 flatPositionEye;

vec3 safeNormalize(vec3 value) {
    float magnitude = length(value);

    return magnitude > 1e-6 ? value / magnitude : vec3(0.0, 0.0, 1.0);
}

void main() {
    vec4 positionWorld = model * vec4(position, 1.0);
    vec4 positionEye = view * positionWorld;
    mat3 normalMatrix = mat3(view * model);

    eyePosition = positionEye.xyz;

    eyeNormal = safeNormalize(normalMatrix * vertexNormal);

    flatNormal = safeNormalize(normalMatrix * vertexNormal);

    worldPosition = positionWorld.xyz;

    patchUV = texCoord;

    flatPositionEye = positionEye.xyz;

    gl_Position = projection * positionEye;
}
