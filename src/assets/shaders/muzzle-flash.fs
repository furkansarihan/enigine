#version 410 core

out vec4 color;

in vec2 UV;

uniform float duration;
uniform float emitDistance;
uniform float particleScale;

vec3 smokeColor = vec3(0.4, 0.4, 0.4);
vec3 fireColor0 = vec3(1, 0.97, 0.59);
vec3 fireColor1 = vec3(0.98, 0.44, 0.18);
vec3 fireColor2 = vec3(0.25, 0.18, 0.16);
float particleRadius = 0.3;

void main()
{
    vec2 center = vec2(0.5, 0.5);
    float dist = length(UV - center);
    float alpha = 1 - smoothstep(0, particleRadius, dist);

    alpha *= duration;
    alpha = clamp(alpha, 0, 1);

    float fragEmitDistance = dist * particleScale + emitDistance;
    if (fragEmitDistance < 0.015) {
        smokeColor = mix(fireColor1, fireColor0, 0.015 / fragEmitDistance);
    } else if (fragEmitDistance < 0.030) {
        smokeColor = mix(fireColor2, fireColor1, 0.015 / fragEmitDistance);
    } else if (fragEmitDistance < 0.045) {
        smokeColor = mix(smokeColor, fireColor2, 0.015 / fragEmitDistance);
    }

    color = vec4(smokeColor, alpha);
}
