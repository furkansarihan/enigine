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
in float _height;
in vec3 _normal;

in vec3 ViewPos;
in vec3 ViewNormal;

uniform vec3 color;

uniform sampler2D texture_diffuse1;

void main()
{
    vec3 MaterialDiffuseColor = texture(texture_diffuse1, TexCoords).rgb;

    gPosition = WorldPos;
    gAlbedo = MaterialDiffuseColor.rgb;
    // TODO: shadow
    gNormalShadow = vec4(normalize(Normal), 1.0);
    gAoRoughMetal.r = 1.0;
    gAoRoughMetal.g = 1.0;
    gAoRoughMetal.b = 1.0;
    gViewPosition = ViewPos;
    gViewNormal = normalize(ViewNormal);
}