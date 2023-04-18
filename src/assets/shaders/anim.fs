#version 410 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Pos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

uniform vec3 lightDir;

void main()
{
    vec3 diffuse = texture(texture_diffuse1, TexCoords).rgb;
    vec3 normal = texture(texture_normal1, TexCoords).rgb;
    
    float distAmbient = dot(lightDir, Normal) + 0.2;
    float distSurface = dot(lightDir, normal) * 2.0 - 1.0;

    vec3 mixed = mix(diffuse * distAmbient, diffuse * distSurface * 1.5, 0.35);

    FragColor = vec4(mixed, 1);
    // FragColor = vec4(vec3(diffuse), 1);
    // FragColor = vec4(vec3(normal), 1);
    // FragColor = vec4(vec3(Normal), 1);
}