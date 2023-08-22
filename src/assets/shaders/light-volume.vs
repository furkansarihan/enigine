#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix;
layout (location = 7) in vec3 alightColor;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 lightColor;

void main()
{
    lightColor = alightColor;
    // gl_Position = projection * view * model * vec4(aPos + aInstancePos, 1.0);
    gl_Position = projection * view * aInstanceMatrix * model * vec4(aPos, 1.0);
}
