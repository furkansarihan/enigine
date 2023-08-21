#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormalShadow;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gAoRoughMetal;

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
in vec2 _heightSlope;

// shadowmap
in vec3 Position_worldspace;
in vec3 LightDirection_cameraspace;
in vec3 EyeDirection_cameraspace;

// TODO: in vertex shader
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

vec3 getNormalFromMap(vec3 tangentNormal)
{
    vec3 Q1  = dFdx(Position_worldspace);
    vec3 Q2  = dFdy(Position_worldspace);
    vec2 st1 = dFdx(_tuv);
    vec2 st2 = dFdy(_tuv);

    vec3 N   = normalize(_normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

vec3 sampleTex(sampler2DArray tex, vec2 hs) {
    float h = hs.x;
    float s = hs.y;

    float textMult = 1.0;

    // TODO: any improvements?
    // TODO: 
    // if (slopeMixer1.mixer == 0 && heightMixer.mixer == 0) {
    //     return samplePBR(vec3(_tuv, textureIndexes[heightMixer.from][slopeMixer1.from]));
    // } else if (slopeMixer1.mixer == 1 && heightMixer.mixer == 0) {
    //     return samplePBR(vec3(_tuv, textureIndexes[heightMixer.from][slopeMixer1.to]));
    // } else if (slopeMixer2.mixer == 0 && heightMixer.mixer == 1) {
    //     return samplePBR(vec3(_tuv, textureIndexes[heightMixer.to][slopeMixer2.from]));
    // } else if (slopeMixer2.mixer == 1 && heightMixer.mixer == 1) {
    //     return samplePBR(vec3(_tuv, textureIndexes[heightMixer.to][slopeMixer2.to]));
    // }
    
    vec3 color1 = texture(tex, vec3(_tuv * textMult, textureIndexes[heightMixer.from][slopeMixer1.from])).rgb;
    vec3 color2 = texture(tex, vec3(_tuv * textMult, textureIndexes[heightMixer.from][slopeMixer1.to])).rgb;
    vec3 mix1 = mix(color1, color2, slopeMixer1.mixer);

    vec3 color3 = texture(tex, vec3(_tuv * textMult, textureIndexes[heightMixer.to][slopeMixer2.from])).rgb;
    vec3 color4 = texture(tex, vec3(_tuv * textMult, textureIndexes[heightMixer.to][slopeMixer2.to])).rgb;
    vec3 mix2 = mix(color3, color4, slopeMixer2.mixer);

    return mix(mix1, mix2, heightMixer.mixer);
}

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

    vec4 ShadowCoord = DepthBiasVP[index] * vec4(Position_worldspace, 1);

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

// TODO: double terrain?
void main()
{
    vec2 hs = _heightSlope;

    if (wireframe) {
        gPosition = Position_worldspace;
        gNormalShadow = vec4(getNormalFromMap(sampleTex(texture_normal1, hs)), 1.0);
        gAlbedo = wireColor;
        gAoRoughMetal.r = sampleTex(texture_ao1, hs).r;
        gAoRoughMetal.g = sampleTex(texture_rough1, hs).r;
        gAoRoughMetal.b = sampleTex(texture_metal1, hs).r;
        return;
    }
    
    float h = hs.x;
    float s = hs.y;

    heightMixer = calculateSampleMixer(heightMixer, h, heightSteps, hTransitions);
    // TODO: 
    // slopeMixer1 = calculateSampleMixer(slopeMixer1, s, slopeStepsList[heightMixer.from], sTransitions[heightMixer.from]);
    // slopeMixer2 = calculateSampleMixer(slopeMixer2, s, slopeStepsList[heightMixer.to], sTransitions[heightMixer.to]);

    // heightMixer.from = 0;
    // heightMixer.to = 0;
    // heightMixer.mixer = 0.0;
    slopeMixer1.from = 2;
    slopeMixer1.to = 2;
    slopeMixer1.mixer = 0.0;
    slopeMixer2.from = 2;
    slopeMixer2.to = 2;
    slopeMixer2.mixer = 0.0;

    gPosition = Position_worldspace;
    gNormalShadow = vec4(getNormalFromMap(sampleTex(texture_normal1, hs)), getVisibility());
    gAlbedo = sampleTex(texture_diffuse1, hs);
    gAoRoughMetal.r = sampleTex(texture_ao1, hs).r;
    gAoRoughMetal.g = sampleTex(texture_rough1, hs).r;
    gAoRoughMetal.b = sampleTex(texture_metal1, hs).r;;

    // TODO: 
    // if (ShowCascade) {
    //     outColor[index] = 0.9;
    // }

    // fog
    // TODO: 
    // float fogFactor = (fogMaxDist - _distance) / (fogMaxDist - fogMinDist);
    // fogFactor = clamp(fogFactor, 0, 1);
    // color = mix(fogColor, outColor, fogFactor);
}
