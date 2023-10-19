#version 410 core

out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 ModelPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

in mat3 ViewTBN;

in mat4 TransformedModel;
uniform mat4 view;
uniform mat4 projection;

struct Material {
    vec4 albedo;
    float roughness;
    float metallic;
    float transmission;
    float opacity;
    float ior;
    // emissive
    vec4 emissiveColor;
    float emissiveStrength;
    // volume
    float thickness;
    // parallax occlusion mapping
    float parallaxMapMidLevel;
    float parallaxMapScale;
    float parallaxMapSampleCount;
    float parallaxMapScaleMode;

    //
    bool albedoMap;
    bool normalMap;
    bool heightMap;
    bool aoMap;
    bool roughMap;
    bool metalMap;
    bool opacityMap;
    bool aoRoughMetalMap;
};

uniform Material material;

// transmission
uniform sampler2D u_TransmissionFramebufferSampler;
uniform vec2 u_TransmissionFramebufferSize;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_metal1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;
uniform sampler2D texture_rough1;
uniform sampler2D texture_ao1;
uniform sampler2D texture_opacity1;
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
    int index = 2;
    if (gl_FragCoord.z < 0) {
    } else if (gl_FragCoord.z < FrustumDistances.x) {
        index = 0;
    } else if (gl_FragCoord.z < FrustumDistances.y) {
        index = 1;
    } else if (gl_FragCoord.z < FrustumDistances.z) {
        index = 2;
    }

    // bias = Bias[index];
    float fragToLight = dot(Normal, lightDirection);
    float bias = max(0.05 * (1.0 - fragToLight), 0.005);  

    mat4 DepthBiasMVP = DepthBiasVP[index] * TransformedModel;
    vec4 ShadowCoord = DepthBiasMVP * vec4(ModelPos, 1);

    float visibility = 1.0;

    float nearestOccluderDist = texture(ShadowMap, vec3(ShadowCoord.xy, index)).x;

    for (int i = 0; i < 8; i++) {
        if (texture(ShadowMap, vec3(ShadowCoord.xy + poissonDisk[i] / 700.0, index)).x < ShadowCoord.z - bias) {
            visibility -= 0.08;
        }
    }

    if (nearestOccluderDist > 0.9) {
        visibility = 1.0;
    }

    return visibility;
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
// Source of the pbr functions below - https://github.com/KhronosGroup/glTF-Sample-Viewer
// ----------------------------------------------------------------------------
float clampedDot(vec3 x, vec3 y)
{
    return clamp(dot(x, y), 0.0, 1.0);
}
// ----------------------------------------------------------------------------
vec3 getVolumeTransmissionRay(vec3 n, vec3 v, float thickness, float ior, mat4 modelMatrix)
{
    // Direction of refracted light.
    vec3 refractionVector = refract(-v, normalize(n), 1.0 / ior);

    // Compute rotation-independant scaling of the model matrix.
    vec3 modelScale;
    modelScale.x = length(vec3(modelMatrix[0].xyz));
    modelScale.y = length(vec3(modelMatrix[1].xyz));
    modelScale.z = length(vec3(modelMatrix[2].xyz));

    // The thickness is specified in local space.
    return normalize(refractionVector) * thickness * modelScale;
}
// ----------------------------------------------------------------------------
float applyIorToRoughness(float roughness, float ior)
{
    // Scale roughness with IOR so that an IOR of 1.0 results in no microfacet refraction and
    // an IOR of 1.5 results in the default amount of microfacet refraction.
    return roughness * clamp(ior * 2.0 - 2.0, 0.0, 1.0);
}
// ----------------------------------------------------------------------------
float V_GGX(float NdotL, float NdotV, float alphaRoughness)
{
    float alphaRoughnessSq = alphaRoughness * alphaRoughness;

    float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
    float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

    float GGX = GGXV + GGXL;
    if (GGX > 0.0)
    {
        return 0.5 / GGX;
    }
    return 0.0;
}
// ----------------------------------------------------------------------------
vec3 getPunctualRadianceTransmission(vec3 normal, vec3 view, vec3 pointToLight, float alphaRoughness,
    vec3 f0, vec3 f90, vec3 baseColor, float ior)
{
    float transmissionRougness = applyIorToRoughness(alphaRoughness, ior);

    vec3 n = normalize(normal);           // Outward direction of surface point
    vec3 v = normalize(view);             // Direction from surface point to view
    vec3 l = normalize(pointToLight);
    vec3 l_mirror = normalize(l + 2.0*n*dot(-l, n));     // Mirror light reflection vector on surface
    vec3 h = normalize(l_mirror + v);            // Halfway vector between transmission light vector and v

    float D = DistributionGGX(n, h, transmissionRougness);
    // TODO: what's the difference?
    float Vis = V_GGX(clampedDot(n, l_mirror), clampedDot(n, v), transmissionRougness);
    // float Vis = GeometrySmith(n, v, l_mirror, transmissionRougness);

    vec3 F = fresnelSchlick(clampedDot(v, h), f0);

    // Transmission BTDF
    return (1.0 - F) * baseColor * D * Vis;
}
// ----------------------------------------------------------------------------
vec3 applyVolumeAttenuation(vec3 radiance, float transmissionDistance, vec3 attenuationColor, float attenuationDistance)
{
    if (attenuationDistance == 0.0)
    {
        // Attenuation distance is +∞ (which we indicate by zero), i.e. the transmitted color is not attenuated at all.
        return radiance;
    }
    else
    {
        // Compute light attenuation using Beer's law.
        vec3 attenuationCoefficient = -log(attenuationColor) / attenuationDistance;
        vec3 transmittance = exp(-attenuationCoefficient * transmissionDistance); // Beer's law
        return transmittance * radiance;
    }
}
// ----------------------------------------------------------------------------
vec3 getTransmissionSample(vec2 fragCoord, float roughness, float ior)
{
    // return textureLod(u_TransmissionFramebufferSampler, fragCoord.xy, 4).rgb;
    // return texture(u_TransmissionFramebufferSampler, fragCoord.xy).rgb;
    float framebufferLod = log2(float(u_TransmissionFramebufferSize.x)) * applyIorToRoughness(roughness, ior);
    framebufferLod = max(framebufferLod, 1); // artifacts when lower than 1
    vec3 transmittedLight = textureLod(u_TransmissionFramebufferSampler, fragCoord.xy, framebufferLod).rgb;
    return transmittedLight;
}
// ----------------------------------------------------------------------------
vec3 getIBLVolumeRefraction(vec3 n, vec3 v, float perceptualRoughness, vec3 baseColor, vec3 f0, vec3 f90,
    vec3 position, mat4 modelMatrix, mat4 viewMatrix, mat4 projMatrix, float ior, float thickness, 
    vec3 attenuationColor, float attenuationDistance)
{
    vec3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, ior, modelMatrix);
    vec3 refractedRayExit = position + transmissionRay;

    // Project refracted vector on the framebuffer, while mapping to normalized device coordinates.
    vec4 ndcPos = projMatrix * viewMatrix * vec4(refractedRayExit, 1.0);
    vec2 refractionCoords = ndcPos.xy / ndcPos.w;
    refractionCoords += 1.0;
    refractionCoords /= 2.0;

    // Sample framebuffer to get pixel the refracted ray hits.
    vec3 transmittedLight = getTransmissionSample(refractionCoords, perceptualRoughness, ior);

    // return transmittedLight;

    vec3 attenuatedColor = applyVolumeAttenuation(transmittedLight, length(transmissionRay), attenuationColor, attenuationDistance);

    // Sample GGX LUT to get the specular component.
    float NdotV = clampedDot(n, v);
    vec2 brdfSamplePoint = clamp(vec2(NdotV, perceptualRoughness), vec2(0.0, 0.0), vec2(1.0, 1.0));
    vec2 brdf = texture(brdfLUT, brdfSamplePoint).rg;
    vec3 specularColor = f0 * brdf.x + f90 * brdf.y;

    return (1.0 - specularColor) * attenuatedColor * baseColor;
}
// ----------------------------------------------------------------------------

void main()
{
    // TODO: preprocessor texture read - material properties
    vec3 albedo;
    float ao, roughness, metallic, opacity;

    if (material.albedoMap) {
        vec4 diffuse = texture(texture_diffuse1, TexCoords);
        albedo = diffuse.rgb;
        albedo *= material.albedo.rgb; // TODO: validate
        opacity = diffuse.a; // TODO: validate
    } else {
        albedo = material.albedo.rgb;
        opacity = 1.0;
    }

    // TODO: validate
    if (material.opacityMap) {
        opacity *= texture(texture_opacity1, TexCoords).r;
    } else {
        opacity *= material.opacity;
    }

    vec3 emissive = material.emissiveColor.xyz * material.emissiveStrength;
    albedo += emissive;

    if (material.aoRoughMetalMap) {
        // TODO: individual present
        vec3 merged = texture(texture_unknown1, TexCoords).rgb;
        ao = 1.0; // TODO: useAOMAP
        roughness = merged.g * material.roughness;
        metallic = merged.b * material.metallic;
    } else {
        if (material.aoMap) {
            ao = texture(texture_ao1, TexCoords).r;
        } else {
            ao = 1.0;
        }
        if (material.roughMap) {
            roughness = texture(texture_rough1, TexCoords).r * material.roughness;
        } else {
            roughness = material.roughness;
        }
        if (material.metalMap) {
            metallic = texture(texture_metal1, TexCoords).r * material.metallic;
        } else {
            metallic = material.metallic;
        }
    }

    vec3 N;
    vec3 V = normalize(camPos - WorldPos);

    if (material.normalMap) {
        vec3 tangentNormal = texture(texture_normal1, TexCoords).xyz * 2.0 - 1.0;
        // TODO: pom tangent and bitangent?
        mat3 TBN = mat3(
            Tangent,
            Bitangent,
            Normal
        );
        N = normalize(TBN * tangentNormal);
    } else {
        N = normalize(Normal);
    }

    // TODO: debug frustumIndex
    // if (frustumIndex == 0) {
    //     albedo = vec3(1, 0, 0);
    // } else if (frustumIndex == 1) {
    //     albedo = vec3(1, 1, 0);
    // } else if (frustumIndex == 2) {
    //     albedo = vec3(1, 0, 1);
    // }

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // NOTE: texture filtering - mipmapping affects this lookup
    vec3 irradiance     = texture(irradianceMap, N).rgb;
    vec3 diffuse        = irradiance * albedo;
    vec3 f_transmission = vec3(0);

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

        // transmission
        // TODO: preprocessor
// #ifdef MATERIAL_TRANSMISSION
    if (material.transmission > 0) {
        // If the light ray travels through the geometry, use the point it exits the geometry again.
        // That will change the angle to the light source, if the material refracts the light ray.
        vec3 n = N;
        vec3 v = V;

        vec3 pointToLight = normalize(lightDirection); // negative?
        vec3 light = lightColor;

        vec3 f90 = vec3(1.0);
        vec3 c_diff = vec3(0.4);
        float thickness = material.thickness;
        float ior = material.ior;

        c_diff = mix(albedo, vec3(0), metallic);

        float perceptualRoughness = roughness;
        // Roughness is authored as perceptual roughness; as is convention,
        // convert to material roughness by squaring the perceptual roughness.
        float alphaRoughness = perceptualRoughness * perceptualRoughness;
        vec3 attenuationColor = vec3(1.0);
        float attenuationDistance = 0.0;

        //
        f_transmission += getIBLVolumeRefraction(
            n, v,
            perceptualRoughness,
            c_diff, F0, f90,
            WorldPos, TransformedModel, view, projection,
            ior, thickness, attenuationColor, attenuationDistance);

        vec3 transmissionRay = getVolumeTransmissionRay(n, v, thickness, ior, TransformedModel);
        pointToLight -= transmissionRay;
        vec3 l = normalize(pointToLight);

        vec3 intensity = lightColor;
        vec3 transmittedLight = intensity * getPunctualRadianceTransmission(n, v, l, alphaRoughness, 
                                                F0, f90, c_diff, ior);

// #ifdef MATERIAL_VOLUME
        transmittedLight = applyVolumeAttenuation(transmittedLight, length(transmissionRay), attenuationColor, attenuationDistance);
// #endif

        f_transmission += transmittedLight;
    }
    }
// #endif

// #ifdef MATERIAL_TRANSMISSION
    if (material.transmission > 0) {
        diffuse = mix(diffuse, f_transmission, material.transmission);
    }
// #endif

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

    vec3 color = ambient + Lo;

// #ifdef MATERIAL_TRANSMISSION
    if (material.transmission > 0) {
        FragColor = vec4(color, opacity);
    }
// #else
    else {
        // TODO: visiblity - when?
        FragColor = vec4(color * getVisibility(), opacity);
    }

    // TODO: only if alphamode is mask
    // const float alphaCutoff = 0.5;
    // if (opacity < alphaCutoff) {
    //     discard;
    // }

    // FragColor = vec4(WorldPos, 1.0);
    // FragColor = vec4(vec3(albedo), 1.0);
    // FragColor = vec4(vec3(metallic), 1.0);
    // FragColor = vec4(vec3(roughness), 1.0);
    // FragColor = vec4(vec3(lightColor), 1.0);
    // FragColor = vec4(vec3(ao), 1.0);
    // FragColor = vec4(diffuse, 1.0);
    // FragColor = vec4(ambient, 1.0);
    // FragColor = vec4(vec3(opacity), 1.0);
    // FragColor = vec4(f_transmission, 1.0);
    // FragColor = vec4(vec3(material.transmission), 1.0);
    // FragColor = vec4(color, 1.0);
    // FragColor = vec4(N, 1.0);
    // FragColor = vec4(irradiance, 1.0);
    // FragColor = vec4(texture(texture_diffuse1, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(texture_normal1, TexCoords).rgb, 1.0);
    // FragColor = vec4(Normal, 1.0);
    // FragColor = vec4(texture(texture_metal1, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(texture_height1, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(texture_rough1, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(texture_ao1, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(texture_opacity1, TexCoords).rgb, 1.0);
    // FragColor = vec4(texture(texture_unknown1, TexCoords).ggg, 1.0);
    // FragColor = vec4(texture(irradianceMap, N).rgb, 1.0);
    // FragColor = vec4(textureLod(prefilterMap, N, 0).rgb, 1.0);
    // FragColor = vec4(texture(brdfLUT, N.xy).rgb, 1.0);
}