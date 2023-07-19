#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 ModelPos;
out vec3 Normal;
out mat4 TransformedModel;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 u_meshOffset;

void main()
{
    TexCoords = aTexCoords * vec2(1.0, 1.0);
    TransformedModel = model * u_meshOffset;
    WorldPos = vec3(TransformedModel * vec4(aPos, 1.0));
    ModelPos = aPos;
    Normal = mat3(transpose(inverse(TransformedModel))) * aNormal;

    gl_Position = projection * view * vec4(WorldPos, 1.0);
}