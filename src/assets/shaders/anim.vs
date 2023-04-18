#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 LightPosition_worldspace;

const int MAX_BONES = 150;
const int MAX_BONE_PER_VERTEX = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 TexCoords;
out vec3 Pos;
out vec3 Normal;

void main()
{
    vec4 totalPosition = vec4(0);
    for(int i = 0 ; i < MAX_BONE_PER_VERTEX; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >= MAX_BONES) 
        {
            totalPosition = vec4(pos,1);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos,1);
        totalPosition += localPosition * weights[i];
        vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * norm;
    }

    mat4 viewModel = view * model;
    gl_Position =  projection * viewModel * totalPosition;

    TexCoords = tex;
    Pos = totalPosition.xyz;
    Normal = mat3(transpose(inverse(model))) * norm;  
}