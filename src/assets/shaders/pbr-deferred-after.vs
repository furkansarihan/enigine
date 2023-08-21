#version 410 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

out vec2 TexCoords;

void main()
{
    TexCoords = uv;
    gl_Position = vec4(position.xy, 0.0, 1.0);
}
