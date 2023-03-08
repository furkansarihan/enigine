#version 410 core

uniform sampler2DArray renderedTexture;
uniform int layer;

in vec2 UV;

out vec4 FragColor;

void main()
{
    vec3 c = texture(renderedTexture, vec3(UV, layer)).rgb;
    FragColor = vec4(vec3(c.r), 1.0);
}
