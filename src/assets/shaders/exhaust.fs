#version 410 core

out vec4 color;

in vec2 UV;

uniform float duration;
uniform float emitDistance;

vec3 smokeColor = vec3(0.3, 0.3, 0.3);
float particleRadius = 0.3;

void main()
{
    vec2 center = vec2(0.5, 0.5);
    float dist = length(UV - center);
    float alpha = 1 - smoothstep(0, particleRadius, dist);

    alpha *= duration;
    alpha = clamp(alpha, 0, 1);

    color = vec4(smokeColor, alpha);
}
