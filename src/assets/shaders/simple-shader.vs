#version 330 core

layout (location = 0) in vec3 vertexPosition_modelspace;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexUV;

out vec3 color;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(vertexPosition_modelspace, 1.0);
    color = vertexPosition_modelspace;
}
