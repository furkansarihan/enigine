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
in float _n;

in vec3 ViewPos;
in vec3 ViewNormal;

uniform sampler2D texture_diffuse1;

uniform vec3 grassColorFactor;

void main()
{
    vec4 MaterialDiffuseColor = texture(texture_diffuse1, TexCoords);

    MaterialDiffuseColor.r *= grassColorFactor.r * max(0.6, _n);
    MaterialDiffuseColor.g *= grassColorFactor.g * max(0.6, _n);
    MaterialDiffuseColor.b *= grassColorFactor.b * max(0.6, _n);

    MaterialDiffuseColor.rgb *= 1.5;

    gPosition = WorldPos;
    gAlbedo = MaterialDiffuseColor.rgb;
    // TODO: shadow
    gNormalShadow = vec4(normalize(Normal), 1.0);
    gAoRoughMetal.r = 1.0 - TexCoords.y;
    gAoRoughMetal.g = 1.0;
    gAoRoughMetal.b = 0.0;
    gViewPosition = ViewPos;
    gViewNormal = normalize(ViewNormal);

    // TODO: fix texture top?
    if (TexCoords.y < 0.1)
        discard;

    // TODO: fix texture alpha edges
    if(MaterialDiffuseColor.a < 0.2)
        discard;
}