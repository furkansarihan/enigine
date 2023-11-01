#version 410 core

out vec4 FragColor;

in vec2 TexCoords;

uniform mat4 TransformedModel;

uniform sampler2D gPosition;
uniform sampler2D gNormalShadow;
uniform sampler2D gAlbedo;
uniform sampler2D gAoRoughMetal;
uniform sampler2D ssaoSampler;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

struct Light {
    vec3 direction;
    vec3 color;
};

uniform Light light;

uniform mat4 view;
uniform vec3 camPos;

// fog
uniform float fogMaxDist;
uniform float fogMinDist;
uniform vec4 fogColor;

const float PI = 3.14159265359;

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
    vec3 WorldPos = texture(gPosition, TexCoords).xyz;
    vec4 normalShadow = texture(gNormalShadow, TexCoords);
    vec3 N = normalShadow.xyz;
    float shadow = normalShadow.w;
    vec3 albedo = pow(texture(gAlbedo, TexCoords).rgb, vec3(2.2));
    vec3 aoRoughMetal = texture(gAoRoughMetal, TexCoords).rgb;
    float ao = aoRoughMetal.r;
    float roughness = aoRoughMetal.g;
    float metallic = aoRoughMetal.b;
    float ssao = texture(ssaoSampler, TexCoords).r;

    ao = min(ao, ssao);

    vec3 V = normalize(camPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // NOTE: texture filtering - mipmapping affects this lookup
    vec3 irradiance     = texture(irradianceMap, N).rgb;
    vec3 diffuse        = irradiance * albedo;

    // reflectance equation
    vec3 Lo = vec3(0.0);
    vec3 L = normalize(light.direction);
    float NdotL = max(dot(N, L), 0.0);

    if (NdotL > 0.0)
    {
        // directional light - sun
        vec3 H = normalize(V + L);
        vec3 radiance = light.color;

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
        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    vec3 R = reflect(-V, N);   
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;   
    vec2 envBRDF  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    
    vec3 ambient = (kD * diffuse + specular) * ao; 

    vec3 color = ambient + Lo * shadow;

    FragColor = vec4(color, 1.0);

    // fog
    float _distance = -(view * vec4(WorldPos, 1.0)).z;
    float fogFactor = (_distance - fogMinDist) / (fogMaxDist - fogMinDist);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    FragColor = mix(FragColor, fogColor, fogFactor);

    // FragColor = vec4(WorldPos, 1.0);
    // FragColor = vec4(albedo, 1.0);
    // FragColor = vec4(N, 1.0);
    // FragColor = vec4(vec3(ao), 1.0);
    // FragColor = vec4(vec3(_distance), 1.0);
    // FragColor = vec4(vec3(roughness), 1.0);
    // FragColor = vec4(vec3(metallic), 1.0);
    // FragColor = vec4(vec3(ssao), 1.0);
    // FragColor = vec4(vec3(lightColor), 1.0);
    // FragColor = vec4(irradiance, 1.0);
    // FragColor = vec4(texture(gPosition, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(gNormal, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(gAlbedo, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(gAoRoughMetal, TexCoords).rrr, 1.0);
    // FragColor = vec4(texture(gAoRoughMetal, TexCoords).ggg, 1.0);
    // FragColor = vec4(texture(gAoRoughMetal, TexCoords).bbb, 1.0);
    // FragColor = vec4(texture(irradianceMap, N).rgb, 1.0);
    // FragColor = vec4(textureLod(prefilterMap, N, 0).rgb, 1.0);
    // FragColor = vec4(texture(brdfLUT, N.xy).rgb, 1.0);
}