#version 410 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform samplerCube irradianceMap;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform float lightPower;

void main()
{
    // Calculate the diffuse color from the texture
    vec3 diffuse = texture(texture_diffuse1, TexCoords).rgb;

    // Calculate the tangent space basis vectors
    vec3 N = normalize(Normal);
    vec3 T = normalize(Tangent - dot(Tangent, N) * N);
    vec3 B = cross(N, T);

    // Calculate the normal from the normal map
    vec3 normalMap = texture(texture_normal1, TexCoords).rgb * 2.0 - 1.0;
    vec3 normal = normalize(T * normalMap.x + B * normalMap.y + N * normalMap.z);

    // Calculate the diffuse lighting contribution from the light direction and vertex normal
    float diffuseLight = max(dot(normal, -lightDir), 0.0) + max(dot(Normal, -lightDir), 0.0);

    // Calculate the ambient lighting contribution
    vec3 ambient = diffuse * 0.5;

    // Combine the diffuse and ambient lighting to get the final color
    vec3 finalColor = 
        ambient + 
        diffuse * diffuseLight * lightColor * lightPower;

    // Set the output color
    FragColor = vec4(finalColor, 1.0);

    // debug
    // FragColor = vec4(vec3(diffuse), 1);
    // FragColor = vec4(vec3(normalMap), 1);
    // FragColor = vec4(vec3(Normal), 1);
    // FragColor = vec4(vec3(Tangent), 1);
    // FragColor = vec4(vec3(Bitangent), 1);
    // FragColor = vec4(N, 1);
    // FragColor = vec4(T, 1);
    // FragColor = vec4(B, 1);
    // FragColor = vec4(vec3(diffuseLight), 1);
}