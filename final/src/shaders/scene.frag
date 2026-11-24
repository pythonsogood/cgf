#version 330 core

in vec3 vWorld;
in vec3 vNormal;
in vec4 vShadow;

out vec4 fragColor;

uniform vec3 uColor;
uniform vec3 uLightDirection;
uniform bool uLightEnabled;
uniform vec4 uViewer; // Direction for parallel rays (w=0), eye position for perspective (w=1).
uniform sampler2D uShadowMap;
uniform bool uShadows;
uniform float uAmbient;
uniform float uSpecularStrength;
uniform float uShininess;

float shadowAmount(vec3 normal) {
    vec3 p = vShadow.xyz / vShadow.w;
    p = p * 0.5 + 0.5;

    if (p.z > 1.0 || p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0) return 0.0;

    float bias = max(0.0015 * (1.0 - dot(normal, uLightDirection)), 0.00035);
    vec2 texel = 1.0 / vec2(textureSize(uShadowMap, 0));
    float amount = 0.0;

    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y)
            amount += p.z - bias > texture(uShadowMap, p.xy + vec2(x, y) * texel).r ? 1.0 : 0.0;

    return amount / 9.0;
}

void main() {
    if (!uLightEnabled) {
        fragColor = vec4(uColor * uAmbient, 1.0);
        return;
    }

    vec3 n = normalize(vNormal);
    vec3 lightDir = normalize(uLightDirection);
    float diffuse = max(dot(n, lightDir), 0.0);
    float visibility = (uShadows && diffuse > 0.0) ? 1.0 - shadowAmount(n) : 1.0;
    float specular = 0.0;

    if (diffuse > 0.0 && uSpecularStrength > 0.0) {
        vec3 viewDir = normalize(uViewer.xyz - vWorld * uViewer.w);
        vec3 halfDir = normalize(lightDir + viewDir);
        specular = pow(max(dot(n, halfDir), 0.0), uShininess) * uSpecularStrength;
    }

    vec3 color = uColor * (uAmbient + diffuse * visibility * (1.0 - uAmbient))
            + vec3(specular * visibility);

    fragColor = vec4(color, 1.0);
}
