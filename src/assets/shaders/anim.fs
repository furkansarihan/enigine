#version 410 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 ModelPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_unknown1;
uniform samplerCube irradianceMap;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform float lightPower;

// shadow
uniform sampler2DArray ShadowMap;
uniform vec4 FrustumDistances;
uniform vec3 camPos;
uniform vec3 CamView;
uniform vec3 Bias;
in mat4 TransformedModel;

layout (std140) uniform matrices {
    mat4 DepthBiasVP[3];
};

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

// TODO: include
float getVisibility()
{
    int index = 2;
    if (gl_FragCoord.z < 0) {
    } else if (gl_FragCoord.z < FrustumDistances.x) {
        index = 0;
    } else if (gl_FragCoord.z < FrustumDistances.y) {
        index = 1;
    } else if (gl_FragCoord.z < FrustumDistances.z) {
        index = 2;
    }

    mat4 DepthBiasMVP = DepthBiasVP[index] * TransformedModel;
    vec4 ShadowCoord = DepthBiasMVP * vec4(ModelPos, 1);

    float visibility = 1.0;

    float nearestOccluderDist = texture(ShadowMap, vec3(ShadowCoord.xy, index)).x;

    for (int i = 0; i < 8; i++) {
        if (texture(ShadowMap, vec3(ShadowCoord.xy + poissonDisk[i] / 700.0, index)).x < ShadowCoord.z - Bias[index]) {
            visibility -= 0.08;
        }
    }

    if (nearestOccluderDist > 0.9) {
        visibility = 1.0;
    }

    return visibility;
}

void main()
{
    // Calculate the diffuse color from the texture
    vec3 diffuse = texture(texture_diffuse1, TexCoords).rgb;
    // TODO: 
    float glosiness = texture(texture_unknown1, TexCoords).g;
    float specular = glosiness;

    // gamma correction
    diffuse = pow(diffuse, vec3(2.2));

    // Calculate the tangent space basis vectors
    vec3 N = normalize(Normal);
    vec3 T = normalize(Tangent - dot(Tangent, N) * N);
    vec3 B = cross(N, T);

    // Calculate the normal from the normal map
    vec3 normalMap = texture(texture_normal1, TexCoords).rgb * 2.0 - 1.0;
    vec3 normal = normalize(T * normalMap.x + B * normalMap.y + N * normalMap.z);

    float cosTheta = clamp(dot(normal, lightDir), 0, 1);

    // Calculate the diffuse lighting contribution from the light direction and vertex normal
    float diffuseLight = cosTheta + max(dot(Normal, lightDir), 0.0);

    // Calculate the ambient lighting contribution
    vec3 ambient = diffuse * 3;

    // Combine the diffuse and ambient lighting to get the final color
    vec3 finalColor = 
        ambient + 
        diffuse * diffuseLight * lightColor * lightPower * cosTheta + 
        specular * diffuseLight * lightColor * lightPower * cosTheta;

    // Set the output color
    FragColor = vec4(finalColor * getVisibility(), 1.0);

    // TODO: debug frustumIndex
    // if (frustumIndex == 0) {
    //     FragColor = vec3(1, 0, 0);
    // } else if (frustumIndex == 1) {
    //     FragColor = vec3(1, 1, 0);
    // } else if (frustumIndex == 2) {
    //     FragColor = vec3(1, 0, 1);
    // }

    // debug
    // FragColor = vec4(vec3(diffuse), 1);
    // FragColor = vec4(vec3(specular), 1);
    // FragColor = vec4(vec3(normalMap), 1);
    // FragColor = vec4(vec3(Normal), 1);
    // FragColor = vec4(vec3(normal), 1);
    // FragColor = vec4(vec3(Tangent), 1);
    // FragColor = vec4(vec3(Bitangent), 1);
    // FragColor = vec4(N, 1);
    // FragColor = vec4(T, 1);
    // FragColor = vec4(B, 1);
    // FragColor = vec4(vec3(diffuseLight), 1);
}