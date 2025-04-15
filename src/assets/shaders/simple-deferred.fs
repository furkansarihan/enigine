#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormalShadow;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gAoRoughMetal;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 u_albedo;
uniform float u_ao = 1.0;
uniform float u_rough = 0.0;
uniform float u_metal = 0.0;

// shadow
uniform sampler2DArray ShadowMap;
uniform vec4 FrustumDistances;
uniform vec3 Bias;
uniform vec3 u_camPosition;
uniform float u_shadowFar;
uniform vec3 lightDirection;

// uniform mat4 projection;
uniform mat4 view;
// uniform mat4 model;

layout (std140) uniform matrices {
    mat4 DepthBiasVP[3];
};

#include <shadowing.fs>

void main()
{
    gPosition = WorldPos;
    gAlbedo = u_albedo;
    // TODO: conditional
    gNormalShadow = vec4(Normal, getVisibility(WorldPos, Normal));
    gAoRoughMetal.r = u_ao;
    gAoRoughMetal.g = u_rough;
    gAoRoughMetal.b = u_metal;
}