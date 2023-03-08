#version 410 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

uniform mat4 MVP;

out vec2 UV;

void main()
{
    UV = uv;
    gl_Position = MVP * vec4(position.xy, 0.0, 1.0);
}
