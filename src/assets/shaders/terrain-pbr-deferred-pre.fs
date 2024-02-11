#version 410 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormalShadow;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gAoRoughMetal;
layout (location = 4) out vec3 gViewPosition;
layout (location = 5) out vec3 gViewNormal;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform bool wireframe;
uniform vec3 wireColor;
uniform float minHeight;
uniform float maxHeight;

uniform sampler2DArray texture_diffuse1;
uniform sampler2DArray texture_normal1;
uniform sampler2DArray texture_ao1;
uniform sampler2DArray texture_rough1;
uniform sampler2DArray texture_height1;
uniform sampler2DArray ShadowMap;

uniform mat4 M;
uniform mat4 view;
uniform mat4 P;

uniform vec3 u_camPosition;
uniform float u_shadowFar;
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

in vec3 ViewPos;
in vec3 ViewNormal;

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
    vec4(0, 0.5, 0.9, 1),
    vec4(0, 0.5, 0.9, 1),
    vec4(0, 0.5, 0.8, 1),
    vec4(0, 0.5, 0.8, 1)
);
mat4 sTransitions = mat4(
    vec4(0, 0.1, 0.1, 0),
    vec4(0, 0.2, 0.1, 0),
    vec4(0, 0.2, 0.1, 0),
    vec4(0, 0.2, 0.1, 0)
);

const int water = 4;
const int sand = 4;
const int stone = 2;
const int grass = 2;
const int rock = 4;
const int snow = 4;

// const int index = 4;
// const int water = index;
// const int sand = index;
// const int stone = index;
// const int grass = index;
// const int rock = index;
// const int snow = index;

// float textMult = 0.125;
float textMult = 1.0 / 4.0;

// TODO: dirt
mat3 textureIndexes = mat3(
    vec3(rock, sand, water),
    vec3(rock, stone, grass),
    vec3(rock, snow, snow)
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

vec3 getNormalFromMap(vec3 T, vec3 tangentNormal, vec3 normal)
{
    vec3 N   = normalize(normal);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

vec3 sampleTex(sampler2DArray tex, vec2 hs, vec2 TexCoords) {
    float h = hs.x;
    float s = hs.y;

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
    
    vec3 color1 = texture(tex, vec3(TexCoords * textMult, textureIndexes[heightMixer.from][slopeMixer1.from])).rgb;

    // return color1;

    vec3 color2 = texture(tex, vec3(TexCoords * textMult, textureIndexes[heightMixer.from][slopeMixer1.to])).rgb;
    vec3 mix1 = mix(color1, color2, slopeMixer1.mixer);

    vec3 color3 = texture(tex, vec3(TexCoords * textMult, textureIndexes[heightMixer.to][slopeMixer2.from])).rgb;
    vec3 color4 = texture(tex, vec3(TexCoords * textMult, textureIndexes[heightMixer.to][slopeMixer2.to])).rgb;
    vec3 mix2 = mix(color3, color4, slopeMixer2.mixer);

    return mix(mix1, mix2, heightMixer.mixer);
}

vec3 sampleTexLod(sampler2DArray tex, vec2 hs, vec2 TexCoords, float lod) {
    float h = hs.x;
    float s = hs.y;

    vec3 color1 = texture(tex, vec3(TexCoords * textMult, textureIndexes[heightMixer.from][slopeMixer1.from]), lod).rgb;
    vec3 color2 = texture(tex, vec3(TexCoords * textMult, textureIndexes[heightMixer.from][slopeMixer1.to]), lod).rgb;
    vec3 mix1 = mix(color1, color2, slopeMixer1.mixer);

    vec3 color3 = texture(tex, vec3(TexCoords * textMult, textureIndexes[heightMixer.to][slopeMixer2.from]), lod).rgb;
    vec3 color4 = texture(tex, vec3(TexCoords * textMult, textureIndexes[heightMixer.to][slopeMixer2.to]), lod).rgb;
    vec3 mix2 = mix(color3, color4, slopeMixer2.mixer);

    return mix(mix1, mix2, heightMixer.mixer);
}

#include <shadowing.fs>

// TODO: include
// ----------------------------------------------------------------------------
// Parallax Occlusion Mapping implementation by Michael Möller for Blender
// https://devtalk.blender.org/t/parallax-occlusion-mapping/15774
// https://archive.blender.org/developer/D9198
// https://archive.blender.org/developer/D9792
// Apache License, Version 2.0

// http://burtleburtle.net/bob/c/lookup3.c
// GPL-2.0-or-later

#define rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

#define final(a, b, c) \
  { \
    c ^= b; \
    c -= rot(b, 14); \
    a ^= c; \
    a -= rot(c, 11); \
    b ^= a; \
    b -= rot(a, 25); \
    c ^= b; \
    c -= rot(b, 16); \
    a ^= c; \
    a -= rot(c, 4); \
    b ^= a; \
    b -= rot(a, 14); \
    c ^= b; \
    c -= rot(b, 24); \
  }

uint hash_uint2(uint kx, uint ky)
{
  uint a, b, c;
  a = b = c = 0xdeadbeefu + (2u << 2u) + 13u;

  b += ky;
  a += kx;
  final(a, b, c);

  return c;
}

float hash_uint2_to_float(uint kx, uint ky)
{
  return float(hash_uint2(kx, ky)) / float(0xFFFFFFFFu);
}

float hash_vec2_to_float(vec2 k)
{
  return hash_uint2_to_float(floatBitsToUint(k.x), floatBitsToUint(k.y));
}

float safe_inverse(float x)
{
  return (x != 0.0) ? 1.0 / x : 0.0;
}

void node_parallax_occlusion_map(vec2 hs,
                                 vec3 co,
                                 float midlevel,
                                 float scale,
                                 float samples_count,
                                 float scale_mode,
                                 vec3 viewPosition,
                                 vec3 viewNormal,
                                 sampler2DArray heightmap,
                                 out vec3 out_co,
                                 out float depth,
                                 out vec3 normal,
                                 out vec3 out_worldposition,
                                 out bool outOfBound)
{
  vec2 dcodx = dFdx(co.xy);
  vec2 dcody = dFdy(co.xy);

  vec3 dPdx = dFdx(viewPosition);
  vec3 dPdy = dFdy(viewPosition);

  mat3 ViewMatrixInverse = mat3(inverse(view));
  vec3 dWdx = mat3(ViewMatrixInverse) * dPdx;
  vec3 dWdy = mat3(ViewMatrixInverse) * dPdy;

  vec3 I = (P[3][3] == 0.0) ? viewPosition : vec3(0.0, 0.0, -1.0);
  vec3 N = normalize(viewNormal);

  float NdotI = dot(N, I);

  vec3 v1 = cross(dPdy, N);
  vec3 v2 = cross(N, dPdx);
  float det = dot(dPdx, v1);
  float f = safe_inverse(dcodx.x * dcody.y - dcodx.y * dcody.x);

  if (scale_mode > 0) {
    scale *= sqrt(abs(det * f));
  }

  float scale_per_sample = -scale / samples_count;
 
  vec2 s = safe_inverse(det) * vec2(dot(v1, I), dot(v2, I));
  vec2 ss = (safe_inverse(det * NdotI) * scale_per_sample) * vec2(dot(v1, I), dot(v2, I));
  vec2 d = -safe_inverse(NdotI) * scale * (dcodx * s.x + dcody * s.y);
  float r = hash_vec2_to_float(co.xy) - 0.5;
  float e = (midlevel - 1.0) * samples_count + r;

  float inverse_samples_count = 1.0 / samples_count;
  vec3 step = inverse_samples_count * vec3(d, -1.0);
  vec3 uvw = vec3(co.xy - (1 - midlevel) * d, 1.0) + r * step;

  ivec2 heightmap_size = textureSize(heightmap, 0).xy;
  vec2 dtcdx = dcodx * heightmap_size;
  vec2 dtcdy = dcody * heightmap_size;
  float lod = max(0.0, 0.5 * log2(min(dot(dtcdx, dtcdx), dot(dtcdy, dtcdy))));

  vec3 step_W = dWdx * ss.x + dWdy * ss.y;
  vec3 W = Position_worldspace + e * step_W;

  /* linear search */
  for (float i = 0.0; i < samples_count; i++) {
    vec3 next = uvw + step;
    vec3 next_W = W + step_W;

    float h = sampleTexLod(heightmap, hs, next.xy, lod).x;
    if (h < next.z) {
      uvw = next;
      W = next_W;
    }
    else {
      break;
    }
  }

  /* binary search */
  for (int i = 0; i < 8; i++) {
    step *= 0.5;
    step_W *= 0.5;
    vec3 next = uvw + step;
    vec3 next_W = W + step_W;
    float h = sampleTexLod(heightmap, hs, next.xy, lod).x;
    if (h < next.z) {
      uvw = next;
      W = next_W;
    }
  }

  out_co = uvw;
  out_worldposition = W;
  vec2 offset = uvw.xy - co.xy;

  float vx = f * (dcody.y * offset.x - dcody.x * offset.y);
  float vy = f * (-dcodx.y * offset.x + dcodx.x * offset.y);
  float vz = scale * (uvw.z - midlevel);

  float Xz = dPdx.z - dot(dPdx, N) * N.z;
  float Yz = dPdy.z - dot(dPdy, N) * N.z;
  depth = -(vx * Xz + vy * Yz + vz * N.z);

  float radius = 1.0 / max(heightmap_size.x, heightmap_size.y);
  float dHdu = sampleTexLod(heightmap, hs, uvw.xy + vec2(radius, 0.0), lod).x -
               sampleTexLod(heightmap, hs, uvw.xy - vec2(radius, 0.0), lod).x;
  float dHdv = sampleTexLod(heightmap, hs, uvw.xy + vec2(0.0, radius), lod).x -
               sampleTexLod(heightmap, hs, uvw.xy - vec2(0.0, radius), lod).x;

  vec3 dPdu = f * (dcody.y * dPdx - dcodx.y * dPdy);
  vec3 dPdv = f * (dcodx.x * dPdy - dcody.x * dPdx);
  vec3 r1 = cross(dPdv, N);
  vec3 r2 = cross(N, dPdu);

  float dr = dot(dPdu, r1) * radius * 2.0;
  vec3 surfgrad = dHdu * r1 + dHdv * r2;
  normal = mat3(inverse(view)) * normalize(abs(dr) * N - (sign(dr) * scale) * surfgrad);

  outOfBound = P[3][3] == 0.0 && NdotI > -vz;
  // outOfBound = (P[3][3] == 0.0) ? -NdotI < uvw.z : NdotI > 0;
}

// ----------------------------------------------------------------------------

// TODO: double terrain?
void main()
{
    vec2 hs = _heightSlope;

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

    // 
    vec2 TexCoords = _tuv;
    vec2 pTexCoords = TexCoords;
    vec3 pNormal = _normal;
    vec3 pViewNormal = ViewNormal;
    vec3 pWorldPos = Position_worldspace;
    vec3 pViewPos = ViewPos + vec3(0, 0, 0);

    // TODO: variable
    // TODO: transition region with parallaxMapScale
    const float maxPomDistance = 25.0;
    bool pomActive = _distance < maxPomDistance;
    if (pomActive) {
        vec3 out_co;
        float depth_offset;
        bool outOfBound;

        float parallaxMapMidLevel = 0.5;
        // TODO: variable - distance based
        float parallaxMapScale = 0.12;
        // TODO: variable - distance based
        float parallaxMapSampleCount = 4.0;
        float parallaxMapScaleMode = 0.0;

        node_parallax_occlusion_map(
            hs,
            vec3(TexCoords, 0.0),
            parallaxMapMidLevel,
            parallaxMapScale,
            parallaxMapSampleCount,
            parallaxMapScaleMode,
            pViewPos,
            pViewNormal,
            texture_height1,
            out_co,                 // out vec3 out_co,
            depth_offset,           // out float depth,
            pNormal,                // out vec3 normal
            pWorldPos,              // out vec3 out_worldposition
            outOfBound
        );

        if (outOfBound) {
            discard;
        } 

        pTexCoords = out_co.xy;
        
        // TODO: disable option
        // TODO: use depth_offset
        vec4 v_clip_coord = (P * view * vec4(pWorldPos, 1));
        float f_ndc_depth = v_clip_coord.z / v_clip_coord.w;
        gl_FragDepth = (1.0 - 0.0) * 0.5 * f_ndc_depth + (1.0 + 0.0) * 0.5;

        // TODO: enable face culling to fix depth artifact
        // if (gl_FragDepth < gl_FragCoord.z - 0.002) {
        //     gl_FragDepth = gl_FragCoord.z;
        // }
    } else {
        // TODO: when exactly should set?
        gl_FragDepth = gl_FragCoord.z;
    }

    if (wireframe) {
        gPosition = pWorldPos;
        gNormalShadow = vec4(pNormal, 1.0);
        gAlbedo = wireColor;
        gAoRoughMetal.r = sampleTex(texture_ao1, hs, pTexCoords).r;
        gAoRoughMetal.g = sampleTex(texture_rough1, hs, pTexCoords).r;
        gAoRoughMetal.b = 0.0; // sampleTex(texture_metal1, hs).r;
        return;
    }

    // TODO: should align with base renderer
    vec3 tangentNormal = sampleTex(texture_normal1, hs, pTexCoords) * 2.0 - 1.0;
    vec3 Q1  = dFdx(pWorldPos);
    vec3 Q2  = dFdy(pWorldPos);
    vec2 st1 = dFdx(pTexCoords);
    vec2 st2 = dFdy(pTexCoords);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);

    vec3 N = getNormalFromMap(T, tangentNormal, pNormal);
    vec3 ViewN = getNormalFromMap(T, tangentNormal, pViewNormal);

    gPosition = pWorldPos;
    gNormalShadow = vec4(N, getVisibility(pWorldPos, pNormal));
    // TODO: why bright?
    gAlbedo = sampleTex(texture_diffuse1, hs, pTexCoords) * 0.8;
    gAoRoughMetal.r = sampleTex(texture_ao1, hs, pTexCoords).r;
    gAoRoughMetal.g = sampleTex(texture_rough1, hs, pTexCoords).r;
    gAoRoughMetal.b = 0.0; // TODO: ?
    gViewPosition = pViewPos;
    gViewNormal = normalize(ViewN);

    // TODO: 
    // if (ShowCascade) {
    //     outColor[index] = 0.9;
    // }
}
