#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;
layout(location = 5) in ivec4 aBoneIds;
layout(location = 6) in vec4 aWeights;

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

out vec3 ViewPos;
out vec3 ViewNormal;
out mat3 ViewTBN;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 u_meshOffset;

void main()
{
    vec4 totalPosition = vec4(0);
    vec3 localNormal = vec3(0);
    vec3 localTangent = vec3(0);
    vec3 localBitangent = vec3(0);
    for(int i = 0; i < MAX_BONE_PER_VERTEX; i++)
    {
        if(aBoneIds[i] == -1) 
            continue;
        if(aBoneIds[i] >= MAX_BONES) 
        {
            totalPosition = vec4(aPos, 1);
            break;
        }
        vec4 localPosition = finalBonesMatrices[aBoneIds[i]] * vec4(aPos, 1);
        totalPosition += localPosition * aWeights[i];
        localNormal += mat3(finalBonesMatrices[aBoneIds[i]]) * aNormal;
        localTangent += mat3(finalBonesMatrices[aBoneIds[i]]) * aTangent;
        localBitangent += mat3(finalBonesMatrices[aBoneIds[i]]) * aBitangent;
    }

    TexCoords = aTexCoords;
    TransformedModel = model * u_meshOffset;
    WorldPos = vec3(TransformedModel * totalPosition);
    ModelPos = totalPosition.xyz;
    ViewPos = (view * vec4(WorldPos, 1)).xyz;

    gl_Position = projection * vec4(ViewPos, 1.0);

    // world space normals
    mat3 normalMatrix = transpose(inverse(mat3(TransformedModel)));
    Normal = normalize(normalMatrix * localNormal);
    Tangent = normalize(normalMatrix * localTangent);
    Bitangent = cross(Normal, Tangent);

    // view space normals
    mat3 viewNormalMatrix = transpose(inverse(mat3(view * TransformedModel)));
    ViewNormal = normalize(viewNormalMatrix * localNormal);

    vec3 viewT = normalize(viewNormalMatrix * localTangent);
    vec3 viewB = cross(ViewNormal, viewT);
    ViewTBN = mat3(viewT, viewB, ViewNormal);
}