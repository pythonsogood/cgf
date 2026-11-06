#version 330 core

uniform int renderMode;
uniform vec3 diffuseColor;
uniform sampler2D imageTexture;

const int flatShading = 0;
const int solidPhong = 1;
const int imagePhong = 2;
const int cloudPhong = 3;
const int unlit = 4;

in vec3 eyePosition;
in vec3 eyeNormal;
in vec3 worldPosition;
in vec2 patchUV;
flat in vec3 flatPositionEye;
flat in vec3 flatNormal;

out vec4 fragmentColor;

const vec3 lightPositionEye = vec3(-0.4, 0.6, 0.4);
const vec3 materialSpecular = vec3(0.8);
const float materialShininess = 80.0;

vec3 safeNormalize(vec3 value) {
    float magnitude = length(value);

    return magnitude > 1e-6 ? value / magnitude : vec3(0.0, 0.0, 1.0);
}

vec3 cloudColor(vec3 p) {
    vec3 color = vec3(0.06, 0.09, 0.22);

    float cloud1 = 1.0 - smoothstep(0.2, 1.1, distance(p, vec3(-0.45, 0.25, 0.30)));
    float cloud2 = 1.0 - smoothstep(0.2, 1.1, distance(p, vec3(0.40, -0.15, 0.35)));
    float cloud3 = 1.0 - smoothstep(0.2, 1.1, distance(p, vec3(0.0, 0.40, -0.35)));

    color = mix(color, vec3(0.12, 0.75, 0.85), cloud1);
    color = mix(color, vec3(0.95, 0.30, 0.20), cloud2);
    color = mix(color, vec3(0.90, 0.75, 0.15), cloud3);

    return color;
}

vec3 phong(vec3 baseColor) {
    vec3 normal = safeNormalize(eyeNormal);
    vec3 viewDirection = safeNormalize(-eyePosition);

    if (dot(normal, viewDirection) < 0.0) {
        normal = -normal;
    }

    vec3 lightDirection = safeNormalize(lightPositionEye - eyePosition);

    float diffuse = max(dot(normal, lightDirection), 0.0);
    vec3 reflected = reflect(-lightDirection, normal);
    float specular = pow(max(dot(reflected, viewDirection), 0.0), materialShininess);

    vec3 color = baseColor * (0.12 + 0.88 * diffuse) + materialSpecular * specular;

    return clamp(color, 0.0, 1.0);
}

void main() {
    if (renderMode == unlit) {
        fragmentColor = vec4(diffuseColor, 1.0);
        return;
    }

    if (renderMode == flatShading) {
        vec3 normal = safeNormalize(flatNormal);
        vec3 lightDirection = safeNormalize(lightPositionEye - flatPositionEye);

        if (dot(normal, -flatPositionEye) < 0.0) {
            normal = -normal;
        }

        float diffuse = max(dot(normal, lightDirection), 0.0);

        fragmentColor = vec4(diffuseColor * (0.12 + 0.88 * diffuse), 1.0);
        return;
    }

    vec3 baseColor = diffuseColor;
    if (renderMode == imagePhong) {
        baseColor = texture(imageTexture, patchUV).rgb;
    } else if (renderMode == cloudPhong) {
        baseColor = cloudColor(worldPosition);
    }

    fragmentColor = vec4(phong(baseColor), 1.0);
}
