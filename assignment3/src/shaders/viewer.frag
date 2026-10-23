#version 330 core

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

in vec3 eyePosition;
in vec3 eyeNormal;
in vec3 interpolatedFaceNormal;
in vec4 gouraudColor;

out vec4 fragmentColor;

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
    if (shadingMode == 0) {
        fragmentColor = vec4(clamp(abs(safeNormalize(interpolatedFaceNormal)), 0.0, 1.0), materialDiffuse.a);
    } else if (shadingMode == 1) {
        fragmentColor = vec4(clamp(gouraudColor.rgb, 0.0, 1.0), materialDiffuse.a);
    } else {
        fragmentColor = vec4(illuminate(eyePosition, eyeNormal), materialDiffuse.a);
    }
}
