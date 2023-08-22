#version 410 core

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormalShadow;
uniform sampler2D gAlbedo;
uniform sampler2D gAoRoughMetal;

// light
in vec3 position;
in vec3 color;
in float radius;
in float linear;
in float quadratic;

uniform vec3 camPos;
uniform vec2 screenSize;

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

void main()
{
    // NDC coordinates (-1 to 1)
    vec2 deferredUV = gl_FragCoord.xy / screenSize;

    vec3 WorldPos = texture(gPosition, deferredUV).xyz;
    vec4 normalShadow = texture(gNormalShadow, deferredUV);
    vec3 N = normalShadow.xyz;
    float shadow = normalShadow.w;
    vec3 albedo = pow(texture(gAlbedo, deferredUV).rgb, vec3(2.2));
    vec3 aoRoughMetal = texture(gAoRoughMetal, deferredUV).rgb;
    float ao = aoRoughMetal.r;
    float roughness = aoRoughMetal.g;
    float metallic = aoRoughMetal.b;

    vec3 V = normalize(camPos - WorldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    vec3 L = normalize(position - WorldPos);
    vec3 H = normalize(V + L);
    float dist = length(position - WorldPos);

    if (dist < radius)
    {
        // float attenuation = 1.0 / (1.0 + linear * dist + quadratic * dist * dist);
        float attenuation = 1.0 - (dist / radius);
        vec3 radiance = color * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
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

    // shadow?

    FragColor = vec4(Lo, 1.0);

    // FragColor = vec4(WorldPos, 1.0);
    // FragColor = vec4(albedo, 1.0);
    // FragColor = vec4(N, 1.0);
    // FragColor = vec4(vec3(shadow), 1.0);
    // FragColor = vec4(vec3(ao), 1.0);
    // FragColor = vec4(vec3(roughness), 1.0);
    // FragColor = vec4(vec3(metallic), 1.0);
    // FragColor = vec4(vec3(color), 1.0);
    // FragColor = vec4(texture(gPosition, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(gNormal, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(gAlbedo, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(gAoRoughMetal, TexCoords).rrr, 1.0);
    // FragColor = vec4(texture(gAoRoughMetal, TexCoords).ggg, 1.0);
    // FragColor = vec4(texture(gAoRoughMetal, TexCoords).bbb, 1.0);
}