#version 410 core

out vec4 color;

in vec2 UV;

uniform float duration;
uniform float emitDistance;

vec3 smokeColor = vec3(0.4, 0.4, 0.4);
vec3 fireColor = vec3(1.0, 0.5, 0.0);
float particleRadius = 0.3;

void main()
{
    vec2 center = vec2(0.5, 0.5);
    float dist = length(UV - center);
    float alpha = 1 - smoothstep(0, particleRadius, dist);

    alpha *= duration;
    alpha = clamp(alpha, 0, 1);

    float fragEmitDistance = dist * 0.1 + emitDistance;
    if (fragEmitDistance < 0.4) {
        smokeColor = mix(smokeColor, fireColor, 0.4 / fragEmitDistance);
    }

    color = vec4(smokeColor, alpha);
}
