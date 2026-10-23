#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 faceNormal;

uniform mat4 view;
uniform mat4 projection;
uniform int shadingMode;
uniform bool perspectiveProjection;
uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec4 materialSpecular;
uniform float materialShininess;
uniform vec3 objectLightEye;
uniform vec3 cameraLightEye;
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;

out vec3 eyePosition;
out vec3 eyeNormal;
out vec3 interpolatedFaceNormal;
out vec4 gouraudColor;

vec3 safeNormalize(vec3 value) {
    float magnitude = length(value);

    return magnitude > 1e-6 ? value / magnitude : vec3(0.0);
}

vec3 illuminate(vec3 positionEye, vec3 normalEye) {
    vec3 normal = safeNormalize(normalEye);
    vec3 viewDirection = perspectiveProjection
        ? safeNormalize(-positionEye) : vec3(0.0, 0.0, 1.0);
    vec3 result = materialAmbient.rgb * lightAmbient.rgb;

    vec3 lightDirections[2] = vec3[2](
            safeNormalize(objectLightEye - positionEye),
            safeNormalize(cameraLightEye - positionEye)
        );

    for (int i = 0; i < 2; ++i) {
        vec3 lightDirection = lightDirections[i];
        float normalLight = max(dot(normal, lightDirection), 0.0);

        if (normalLight > 0.0) {
            result += materialDiffuse.rgb * lightDiffuse.rgb * normalLight;
            vec3 reflectionDirection = reflect(-lightDirection, normal);
            float specularAlignment = dot(
                    safeNormalize(reflectionDirection), viewDirection
                );

            if (specularAlignment > 0.0) {
                float specularFactor = pow(
                        specularAlignment, max(materialShininess, 0.0)
                    );

                result += materialSpecular.rgb * lightSpecular.rgb * specularFactor;
            }
        }
    }

    return clamp(result, 0.0, 1.0);
}

void main() {
    vec4 positionEye = view * vec4(position, 1.0);
    eyePosition = positionEye.xyz;
    eyeNormal = mat3(view) * vertexNormal;
    interpolatedFaceNormal = faceNormal;
    gl_Position = projection * positionEye;

    if (shadingMode == 1) {
        gouraudColor = vec4(illuminate(positionEye.xyz, eyeNormal), materialDiffuse.a);
    } else {
        gouraudColor = vec4(0.0);
    }
}
