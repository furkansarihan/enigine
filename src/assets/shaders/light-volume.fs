#version 410 core

out vec4 color;

uniform vec4 DiffuseColor;

void main()
{        
    color = DiffuseColor;
}
