#version 410 core

out vec4 color;

in vec2 UV;

uniform float duration;
uniform float emitDistance;

uniform sampler2D texture_diffuse1;

void main()
{
    color = texture(texture_diffuse1, UV);

    color.xyz *= vec3(1.5, 1.5, 1.5);
}
