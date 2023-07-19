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
uniform mat4 u_meshOffset;

const int MAX_BONES = 200;
const int MAX_BONE_PER_VERTEX = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 ModelPos;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;
out mat4 TransformedModel;

void main()
{
    vec4 totalPosition = vec4(0);
    vec3 localNormal = vec3(0);
    vec3 localTangent = vec3(0);
    vec3 localBitangent = vec3(0);
    for(int i = 0; i < MAX_BONE_PER_VERTEX; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >= MAX_BONES) 
        {
            totalPosition = vec4(pos, 1);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos, 1);
        totalPosition += localPosition * weights[i];
        localNormal += mat3(finalBonesMatrices[boneIds[i]]) * norm;
        localTangent += mat3(finalBonesMatrices[boneIds[i]]) * tangent;
        localBitangent += mat3(finalBonesMatrices[boneIds[i]]) * bitangent;
    }

    TexCoords = tex;
    TransformedModel = model * u_meshOffset;
    WorldPos = vec3(TransformedModel * totalPosition);
    ModelPos = totalPosition.xyz;
    mat3 invTrModel = mat3(transpose(inverse(TransformedModel)));
    Normal = invTrModel * normalize(localNormal);
    Tangent = invTrModel * normalize(localTangent);
    Bitangent = invTrModel * normalize(localBitangent);

    gl_Position = projection * view * vec4(WorldPos, 1);
}