#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormalShadow;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gAoRoughMetal;
layout (location = 4) out vec3 gViewPosition;
layout (location = 5) out vec3 gViewNormal;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 ModelPos;
in vec3 Normal;
in vec3 ViewPos;
in vec3 ViewNormal;

in mat3 TBN;
in mat3 ViewTBN;
in vec3 TangentViewerPos;
in vec3 TangentFragPos;

in mat4 TransformedModel;

struct Material {
    vec3 albedo;
    float ao;
    float roughnessFactor;
    float metallicFactor;
    float opacity;
    float transmission_factor;
    //
    bool normalMap;
    bool heightMap;
    bool aoMap;
    bool roughMap;
    bool metalMap;
    bool aoRoughMetalMap;
};

uniform Material material;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_metal1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;
uniform sampler2D texture_rough1;
uniform sampler2D texture_ao1;
uniform sampler2D texture_unknown1;

// sun
uniform vec3 lightDirection;

// shadow
uniform sampler2DArray ShadowMap;
uniform vec4 FrustumDistances;
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

// TODO: variable parameters
// TODO: height or 1 - height
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    float heightScale = 0.05;
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = 1.0 - texture(texture_height1, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = 1.0 - texture(texture_height1, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = 1.0 - texture(texture_height1, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main()
{
    vec2 pTexCoords;
    if (material.heightMap) {
        vec3 viewDir = normalize(TangentViewerPos - TangentFragPos);
        pTexCoords = ParallaxMapping(TexCoords, viewDir);
        // TODO:
        // if(pTexCoords.x > 1.0 || pTexCoords.y > 1.0 || pTexCoords.x < 0.0 || pTexCoords.y < 0.0)
        //     discard;
    } else {
        pTexCoords = TexCoords;
    }

    // TODO: preprocessor texture read - material properties
    vec3 albedo = texture(texture_diffuse1, pTexCoords).rgb;
    float metallic, roughness, ao;

    // TODO: merge detection
    if (material.aoRoughMetalMap) {
        // TODO: individual present
        vec3 merged = texture(texture_unknown1, pTexCoords).rgb;
        ao = merged.r;
        roughness = merged.g;
        metallic = merged.b;
    } else {
        if (material.aoMap) {
            ao = texture(texture_ao1, pTexCoords).r;
        } else {
            // TODO:
            ao = 1;
        }
        if (material.roughMap) {
            roughness = texture(texture_rough1, pTexCoords).r;
        } else {
            roughness = material.roughnessFactor;
        }
        if (material.metalMap) {
            metallic = texture(texture_metal1, pTexCoords).r;
        } else {
            metallic = material.metallicFactor;
        }
    }

    vec3 N;
    vec3 ViewN;
    if (material.normalMap) {
        vec3 tangentNormal = texture(texture_normal1, pTexCoords).xyz * 2.0 - 1.0;
        N = normalize(TBN * tangentNormal);
        ViewN = normalize(ViewTBN * tangentNormal);
    } else {
        N = normalize(Normal);
        ViewN = normalize(ViewNormal);
    }

    gPosition = WorldPos;
    gNormalShadow = vec4(N, getVisibility());
    gAlbedo = albedo;
    gAoRoughMetal.r = ao;
    gAoRoughMetal.g = roughness;
    gAoRoughMetal.b = metallic;
    gViewPosition = ViewPos;
    gViewNormal = ViewN;

    // TODO: debug frustumIndex
    // if (frustumIndex == 0) {
    //     albedo = vec3(1, 0, 0);
    // } else if (frustumIndex == 1) {
    //     albedo = vec3(1, 1, 0);
    // } else if (frustumIndex == 2) {
    //     albedo = vec3(1, 0, 1);
    // }

    // gAlbedo = vec3(0, 0, 0);
    // gAlbedo = WorldPos;
    // gAlbedo = vec3(albedo);
    // gAlbedo = vec3(metallic);
    // gAlbedo = vec3(roughness);
    // gAlbedo = vec3(ao);
    // gAlbedo = N;
    // gAlbedo = irradiance;
    // gAlbedo = texture(texture_diffuse1, pTexCoords).rgb;
    // gAlbedo = texture(texture_normal1, pTexCoords).rgb;
    // gAlbedo = Normal;
    // gAlbedo = texture(texture_metal1, pTexCoords).rgb;
    // gAlbedo = texture(texture_height1, pTexCoords).rgb;
    // gAlbedo = texture(texture_rough1, pTexCoords).rgb;
    // gAlbedo = texture(texture_ao1, pTexCoords).rgb;
    // gAlbedo = texture(texture_unknown1, pTexCoords).ggg;
    // gAlbedo = texture(irradianceMap, N).rgb;
    // gAlbedo = textureLod(prefilterMap, N, 0).rgb;
    // gAlbedo = texture(brdfLUT, N.xy).rgb;
}