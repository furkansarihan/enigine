#version 410 core

out vec4 FragColor;

in vec2 TexCoords;
in float _height;
in vec3 _normal;
in float _n;

uniform sampler2D texture_diffuse1;

uniform vec3 grassColorFactor;

void main()
{
    vec4 MaterialDiffuseColor = texture(texture_diffuse1, TexCoords);

    MaterialDiffuseColor.rgb *= (1 - TexCoords.y + 0.5);

    MaterialDiffuseColor.r *= grassColorFactor.r * max(0.6, _n);
    MaterialDiffuseColor.g *= grassColorFactor.g * max(0.6, _n);
    MaterialDiffuseColor.b *= grassColorFactor.b * max(0.6, _n);

    MaterialDiffuseColor.rgb *= 1.5;

    // gamma correction
    MaterialDiffuseColor.rgb = pow(MaterialDiffuseColor.rgb, vec3(2.2));
    FragColor = MaterialDiffuseColor;

    // TODO: fix texture top?
    if (TexCoords.y < 0.1)
        discard;

    // TODO: fix texture alpha edges
    if(FragColor.a < 0.2)
        discard;
}