#version 410 core

out vec4 color;

in vec3 lightColor;

void main()
{        
    color = vec4(lightColor, 1.0);
}
