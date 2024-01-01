#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormalShadow;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gAoRoughMetal;
layout (location = 4) out vec3 gViewPosition;
layout (location = 5) out vec3 gViewNormal;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

in vec3 ViewPos;
in vec3 ViewNormal;

uniform vec3 u_albedo;
uniform float u_ao = 1.0;
uniform float u_rough = 0.0;
uniform float u_metal = 0.0;

void main()
{
    gPosition = WorldPos;
    gAlbedo = u_albedo;
    // TODO: shadow
    gNormalShadow = vec4(normalize(Normal), 1.0);
    gAoRoughMetal.r = u_ao;
    gAoRoughMetal.g = u_rough;
    gAoRoughMetal.b = u_metal;
    gViewPosition = ViewPos;
    gViewNormal = normalize(ViewNormal);
}