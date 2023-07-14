#version 410 core

out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 ModelPos;
in vec3 Normal;

// material parameters
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;
uniform bool mergedPBRTextures;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_metal1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;
uniform sampler2D texture_rough1;
uniform sampler2D texture_ao1;
uniform sampler2D texture_unknown1;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

// sun
uniform vec3 lightDirection;
uniform vec3 lightColor;

uniform vec3 camPos;

// shadow
uniform sampler2DArray ShadowMap;
uniform vec4 FrustumDistances;
uniform vec3 CamView;
uniform vec3 Bias;
uniform mat4 model;

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

const float PI = 3.14159265359;

// TODO: include
float getVisibility()
{
    // shadow
    vec3 v = WorldPos - camPos;
    float fragToCamDist = dot(v, CamView);

    int index = 4;
    if (fragToCamDist < 0) {
    } else if (fragToCamDist < FrustumDistances.x) {
        index = 0;
    } else if (fragToCamDist < FrustumDistances.y) {
        index = 1;
    } else if (fragToCamDist < FrustumDistances.z) {
        index = 2;
    }

    mat4 DepthBiasMVP = DepthBiasVP[index] * model;
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

// TODO: any other way?
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(texture_normal1, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
void main()
{
    vec3 albedo     = pow(texture(texture_diffuse1, TexCoords).rgb, vec3(2.2));
    float metallic, roughness, ao;

    // TODO: merge detection
    // ao-rough-metal
    if (mergedPBRTextures) {
        vec3 merged = texture(texture_unknown1, TexCoords).rgb;
        ao = merged.r;
        roughness = merged.g;
        metallic = merged.b;
    } else {
        metallic = texture(texture_metal1, TexCoords).r;
        roughness = texture(texture_rough1, TexCoords).r;
        ao = texture(texture_ao1, TexCoords).r;
    }

    // TODO: variable ao-rough-metal

    // vec3 N = normalize(Normal);
    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    {
        // TODO: light sources
        // calculate per-light radiance
        // vec3 L = normalize(lightPositions[i] - WorldPos);
        // vec3 H = normalize(V + L);
        // float distance = length(lightPositions[i] - WorldPos);
        // float attenuation = 1.0 / (distance * distance);
        // vec3 radiance = lightColors[i] * attenuation;

        // sun
        vec3 L = normalize(lightDirection);
        vec3 H = normalize(V + L);
        vec3 radiance = lightColor;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    // vec3 ambient = vec3(0.03) * albedo * ao;

    // ----
    vec3 R = reflect(-V, N);   
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    
    // NOTE: texture filtering - mipmapping affects this lookup
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * albedo;
    
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;   
    vec2 envBRDF  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    
    vec3 ambient = (kD * diffuse + specular) * ao; 

    // ----
    // vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness); 
    // vec3 kD = vec3(1.0) - kS;
    // kD *= 1.0 - metallic;
    // vec3 irradiance = texture(irradianceMap, N).rgb;
    // vec3 diffuse    = irradiance * albedo;
    // vec3 ambient    = (kD * diffuse) * ao; 

    vec3 color = ambient + Lo;

    FragColor = vec4(color * getVisibility(), 1.0);

    // FragColor = vec4(WorldPos, 1.0);
    // FragColor = vec4(vec3(albedo), 1.0);
    // FragColor = vec4(vec3(metallic), 1.0);
    // FragColor = vec4(vec3(roughness), 1.0);
    // FragColor = vec4(vec3(ao), 1.0);
    // FragColor = vec4(N, 1.0);
    // FragColor = vec4(irradiance, 1.0);
    // FragColor = vec4(texture(texture_diffuse1, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(texture_normal1, TexCoords).rgb, 1.0);
    // FragColor = vec4(Normal, 1.0);
    // FragColor = vec4(texture(texture_metal1, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(texture_height1, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(texture_rough1, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(texture_ao1, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(texture_unknown1, TexCoords).ggg, 1.0);
    // FragColor = vec4(texture(irradianceMap, N).rgb, 1.0);
    // FragColor = vec4(textureLod(prefilterMap, N, 0).rgb, 1.0);
    // FragColor = vec4(texture(brdfLUT, N.xy).rgb, 1.0);
}