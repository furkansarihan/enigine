#version 410 core

uniform sampler2D renderedTexture;

in vec2 UV;

out vec4 FragColor;

void main()
{
    vec3 c = texture(renderedTexture, UV).rgb;
    FragColor = vec4(vec3(c.r), 1.0);
}
