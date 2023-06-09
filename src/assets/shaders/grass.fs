#version 410 core

out vec4 FragColor;

in vec2 TexCoords;
in float _height;
in vec3 _normal;

uniform sampler2D texture_diffuse1;

void main()
{
    vec4 MaterialDiffuseColor = texture(texture_diffuse1, TexCoords);
    // gamma correction
    MaterialDiffuseColor.rgb = pow(MaterialDiffuseColor.rgb, vec3(2.2));
    FragColor = MaterialDiffuseColor;

    // TODO: better way?
    if(FragColor.a < 0.55)
        discard;
}