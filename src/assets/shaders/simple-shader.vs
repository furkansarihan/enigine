#version 410 core

layout (location = 0) in vec3 vertexPosition_modelspace;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexUV;

uniform mat4 MVP;
uniform mat4 u_meshOffset;

void main()
{
    gl_Position = MVP * u_meshOffset * vec4(vertexPosition_modelspace, 1.0);
}
