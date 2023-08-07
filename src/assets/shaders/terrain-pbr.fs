#version 410 core

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform bool wireframe;
uniform vec3 wireColor;
uniform float fogMaxDist;
uniform float fogMinDist;
uniform vec4 fogColor;
uniform float minHeight;
uniform float maxHeight;

uniform sampler2DArray texture_diffuse1;
uniform sampler2DArray texture_metal1;
uniform sampler2DArray texture_normal1;
uniform sampler2DArray texture_height1;
uniform sampler2DArray texture_rough1;
uniform sampler2DArray texture_ao1;
uniform sampler2DArray ShadowMap;

uniform mat4 M;
uniform mat4 V;

uniform vec3 CamPos;
uniform vec3 CamView;
uniform vec4 FrustumDistances;
uniform bool ShowCascade;
uniform vec3 Bias;

layout (std140) uniform matrices {
    mat4 DepthBiasVP[3];
};

in float _height; // based on world coorinates
in vec2 _tuv; // texture coordinates
in float _distance; // vertex distance to the camera
in vec3 _normal; // vertex normal

// shadowmap
in vec3 Position_worldspace;
in vec3 LightDirection_cameraspace;
in vec3 EyeDirection_cameraspace;

out vec4 color;

struct Mixer
{
    int from;
    int to;
    float mixer;
} heightMixer, slopeMixer1, slopeMixer2;

vec4 heightSteps = vec4(-1, 0.001, 0.7, 1);
vec4 hTransitions = vec4(0, 0.002, 0.2, 0);
mat4 slopeStepsList = mat4(
    vec4(0, 0.1, 0.999, 1),
    vec4(0, 0.5, 0.9, 1),
    vec4(0, 0.5, 0.8, 1),
    vec4(0, 0.5, 0.8, 1)
);
mat4 sTransitions = mat4(
    vec4(0, 0.1, 0.001, 0),
    vec4(0, 0.2, 0.1, 0),
    vec4(0, 0.2, 0.1, 0),
    vec4(0, 0.2, 0.1, 0)
);

const int water = 0;
const int sand = 1;
const int stone = 4;
const int grass = 1;
const int rock = 5;
const int snow = 5;

// TODO: dirt
mat3 textureIndexes = mat3(
    vec3(rock, sand, water),
    vec3(rock, stone, grass),
    vec3(rock, snow, snow)
);

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

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i){
    vec4 seed4 = vec4(seed,i);
    float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

vec2 calculateHeightSlopeVector(float slope) {
    float maxHeightGap = maxHeight - minHeight;
    
    float h = _height / maxHeightGap;

    return vec2(h, slope);
}

Mixer calculateSampleMixer(Mixer result, float value, vec4 steps, vec4 transitions) {
    for (int i = 0; i < 3; i++) {
        if (value >= steps[i] && value < steps[i + 1]) {
            if (i != 0 && value < steps[i] + transitions[i]) {
                result.from = i - 1;
                result.to = i;
                result.mixer = (transitions[i] + value - steps[i]) / (transitions[i] * 2);
            } else if (i != 3 && value > steps[i + 1] - transitions[i + 1]) {
                result.from = i;
                result.to = i + 1;
                result.mixer = (value - (steps[i + 1] - transitions[i + 1])) / (transitions[i + 1] * 2);
            } else {
                result.from = i;
                result.to = i;
                result.mixer = 1.0;
            }

            break;
        }
    }

    return result;
}

// TODO: include
// ----------------------------------------------------------------------------
// PBR-------------------------------------------------------------------------
// ----------------------------------------------------------------------------
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

const float PI = 3.14159265359;

vec3 getNormalFromMap(vec3 depth)
{
    vec3 tangentNormal = texture(texture_normal1, depth).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(Position_worldspace);
    vec3 Q2  = dFdy(Position_worldspace);
    vec2 st1 = dFdx(depth.xy);
    vec2 st2 = dFdy(depth.xy);

    vec3 N   = normalize(_normal);
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
// ----------------------------------------------------------------------------
vec3 samplePBR(vec3 depth) {
    vec3 albedo     = pow(texture(texture_diffuse1, depth).rgb, vec3(2.2));
    float metallic  = texture(texture_metal1, depth).r;
    // float metallic = 0.3;
    float roughness = texture(texture_rough1, depth).r;
    float ao        = texture(texture_ao1, depth).r;
    // float ao = 1.0;

    // vec3 N = normalize(Normal);
    vec3 N = getNormalFromMap(depth);
    vec3 V = normalize(CamPos - Position_worldspace);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    // TODO: light sources
    // vec3 lightPositions[1] = vec3[](
    //     vec3(0.0, 0.0, 10.0)
    // );
    // vec3 lightColors[1] = vec3[]( 
    //     vec3(350.0, 410.0, 458.0)
    // );

    // calculate radiance
    {
        vec3 L = normalize(lightDirection);
        vec3 H = normalize(V + L);
        vec3 radiance = lightColor;

        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
            
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;  
        float NdotL = max(dot(N, L), 0.0);        
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // ----
    vec3 R = reflect(-V, N);   
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * albedo;
    
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;   
    vec2 envBRDF  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    
    vec3 ambient = (kD * diffuse + specular) * ao; 

    vec3 color = ambient + Lo;

    return color;
}
// ----------------------------------------------------------------------------
// PBR-------------------------------------------------------------------------
// ----------------------------------------------------------------------------

vec3 sampleTexture(vec2 hs) {
    float h = hs.x;
    float s = hs.y;

    heightMixer = calculateSampleMixer(heightMixer, h, heightSteps, hTransitions);
    slopeMixer1 = calculateSampleMixer(slopeMixer1, s, slopeStepsList[heightMixer.from], sTransitions[heightMixer.from]);
    slopeMixer2 = calculateSampleMixer(slopeMixer2, s, slopeStepsList[heightMixer.to], sTransitions[heightMixer.to]);

    // TODO: any improvements?
    // if (slopeMixer1.mixer == 0 && heightMixer.mixer == 0) {
    //     return samplePBR(vec3(_tuv, textureIndexes[heightMixer.from][slopeMixer1.from]));
    // } else if (slopeMixer1.mixer == 1 && heightMixer.mixer == 0) {
    //     return samplePBR(vec3(_tuv, textureIndexes[heightMixer.from][slopeMixer1.to]));
    // } else if (slopeMixer2.mixer == 0 && heightMixer.mixer == 1) {
    //     return samplePBR(vec3(_tuv, textureIndexes[heightMixer.to][slopeMixer2.from]));
    // } else if (slopeMixer2.mixer == 1 && heightMixer.mixer == 1) {
    //     return samplePBR(vec3(_tuv, textureIndexes[heightMixer.to][slopeMixer2.to]));
    // }

    float textMult = 1.0;
    
    vec3 color1 = samplePBR(vec3(_tuv * textMult, textureIndexes[heightMixer.from][slopeMixer1.from]));
    vec3 color2 = samplePBR(vec3(_tuv * textMult, textureIndexes[heightMixer.from][slopeMixer1.to]));
    vec3 mix1 = mix(color1, color2, slopeMixer1.mixer);

    vec3 color3 = samplePBR(vec3(_tuv * textMult, textureIndexes[heightMixer.to][slopeMixer2.from]));
    vec3 color4 = samplePBR(vec3(_tuv * textMult, textureIndexes[heightMixer.to][slopeMixer2.to]));
    vec3 mix2 = mix(color3, color4, slopeMixer2.mixer);

    return mix(mix1, mix2, heightMixer.mixer);
}

void main()
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
    float fragToLight = dot(_normal, lightDirection);
    float bias = max(0.05 * (1.0 - fragToLight), 0.005);

    vec4 ShadowCoord = DepthBiasVP[index] * vec4(Position_worldspace, 1);

    // float bias = 0.05 * tan(acos(cosTheta));
    // bias = clamp(bias, 0, 0.05);
    // bias = 0.1;

    float visibility = 1.0;

    // 1. sampler2D basic
    float nearestOccluderDist = texture(ShadowMap, vec3(ShadowCoord.xy, index)).x;
    // vec4 outColor = vec4(vec3(nearestOccluderDist), 1.0);
    // float fragDist = ShadowCoord.z;
    // 
    // if (nearestOccluderDist < fragDist - bias){
    //     visibility = 0.5;
    // }

    // 2. sampler2DShadow
    // Compute the depth comparison value
    // float shadowValue = texture(ShadowMap, ShadowCoord.xyz, bias);
    // // Use the depth comparison value to determine if the fragment is in shadow
    // if (ShadowCoord.w > shadowValue)
    // {
    //     visibility = 0.5;
    // }

    // 3. Poission sampling
    for (int i = 0; i < 8; i++) {
        // 1. simple
        if (texture(ShadowMap, vec3(ShadowCoord.xy + poissonDisk[i] / 700.0, index)).x < ShadowCoord.z - bias) {
            visibility -= 0.08;
        }

        // 2. simple2
        // use either :
        //  - Always the same samples.
        //    Gives a fixed pattern in the shadow, but no noise
        // int index = i;
        //  - A random sample, based on the pixel's screen location. 
        //    No banding, but the shadow moves with the camera, which looks weird.
        // int index = int(16.0 * random(gl_FragCoord.xyy, i)) % 16;
        //  - A random sample, based on the pixel's position in world space.
        //    The position is rounded to the millimeter to avoid too much aliasing
        // int index = int(16.0 * random(floor(Position_worldspace.xyz * 1000.0), i)) % 16;

        // being fully in the shadow will eat up 4*0.2 = 0.8
        // 0.2 potentially remain, which is quite dark.
        // visibility -= 0.2 * (1.0 - texture(ShadowMap, vec3(ShadowCoord.xy + poissonDisk[index] / 700.0, 2)).x);
    }

    // Z comparision doesn't work expected when two distance is too close to each other
    // So, we're making fragment fully visible when it's far enough
    if (nearestOccluderDist > 0.9) {
        visibility = 1.0;
    }

    vec4 outColor = vec4(0.8, 0.8, 0.8, 1.0);
    if (wireframe) {
        float distanceRate = (_distance / 10000);
        float normalRate = 1 - distanceRate;
        outColor = vec4(wireColor, 1 * normalRate);
    } else {
        float slope = dot(vec3(0, 1, 0), _normal);
        vec2 hs = calculateHeightSlopeVector(slope);
        outColor = vec4(sampleTexture(hs), 1);
    }

    // TODO: debug frustumIndex
    // if (frustumIndex == 0) {
    //     outColor = vec3(1, 0, 0);
    // } else if (frustumIndex == 1) {
    //     outColor = vec3(1, 1, 0);
    // } else if (frustumIndex == 2) {
    //     outColor = vec3(1, 0, 1);
    // }

    outColor.xyz *= visibility;

    if (ShowCascade) {
        outColor[index] = 0.9;
    }

    // fog
    float fogFactor = (fogMaxDist - _distance) / (fogMaxDist - fogMinDist);
    fogFactor = clamp(fogFactor, 0, 1);
    color = mix(fogColor, outColor, fogFactor);
}
