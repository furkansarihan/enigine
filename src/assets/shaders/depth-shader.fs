#version 410 core

layout(location = 0) out float fragmentdepth;

void main()
{
    fragmentdepth = gl_FragCoord.z;
}
