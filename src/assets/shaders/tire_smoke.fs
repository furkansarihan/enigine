#version 410 core

out vec4 color;

in vec2 UV;

uniform float duration;
uniform float emitDistance;

float particleRadius = 0.3;

void main()
{
    vec2 center = vec2(0.5, 0.5);
    float dist = length(UV - center);
    float radius = clamp(particleRadius * emitDistance, 0.15, 0.3);
    float alpha = 1 - smoothstep(0, radius, dist);

    alpha *= duration;
    alpha = clamp(alpha, 0, 1);

    float smoke = 1.;
    smoke *= emitDistance;
    smoke = clamp(smoke, 0.2, 0.8);

    if (alpha < 0.01)
        gl_FragDepth = 1.0;
    else
        gl_FragDepth = gl_FragCoord.z;

    color = vec4(vec3(smoke), alpha);
}
