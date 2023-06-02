#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 LightPosition_worldspace;

out vec2 TexCoords;
out vec3 Pos;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;

void main()
{
    mat4 viewModel = view * model;
    gl_Position = projection * viewModel * vec4(pos, 1);

    TexCoords = tex;
    Pos = pos;
    Normal = norm;
    Tangent = tangent;
    Bitangent = bitangent;
}
