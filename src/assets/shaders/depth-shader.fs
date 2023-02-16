#version 330 core

layout(location = 0) out float fragmentdepth;

uniform vec4 DiffuseColor;

void main()
{
    fragmentdepth = gl_FragCoord.z;
}
