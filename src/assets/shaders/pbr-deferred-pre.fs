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
in vec3 Tangent;
in vec3 Bitangent;
in vec3 Normal;
in vec3 ViewPos;
in vec3 ViewNormal;

in mat3 ViewTBN;

in mat4 TransformedModel;

struct Material {
    vec4 albedo;
    float roughness;
    float metallic;
    // emissive
    vec4 emissiveColor;
    float emissiveStrength;
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

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// sun
uniform vec3 lightDirection;

// shadow
uniform sampler2DArray ShadowMap;
uniform vec4 FrustumDistances;
uniform vec3 Bias;
uniform vec3 u_camPosition;
uniform float u_shadowFar;

layout (std140) uniform matrices {
    mat4 DepthBiasVP[3];
};

#include <shadowing.fs>

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

void node_parallax_occlusion_map(vec3 co,
                                 float midlevel,
                                 float scale,
                                 float samples_count,
                                 float scale_mode,
                                 vec3 viewPosition,
                                 vec3 viewNormal,
                                 sampler2D heightmap,
                                 out vec3 out_co,
                                 out float depth,
                                 out vec3 normal,
                                 out vec3 out_worldposition)
{
  vec2 dcodx = dFdx(co.xy);
  vec2 dcody = dFdy(co.xy);

  vec3 dPdx = dFdx(viewPosition);
  vec3 dPdy = dFdy(viewPosition);

  mat3 ViewMatrixInverse = mat3(inverse(view));
  vec3 dWdx = mat3(ViewMatrixInverse) * dPdx;
  vec3 dWdy = mat3(ViewMatrixInverse) * dPdy;

  vec3 I = (projection[3][3] == 0.0) ? viewPosition : vec3(0.0, 0.0, -1.0);
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

  ivec2 heightmap_size = textureSize(heightmap, 0);
  vec2 dtcdx = dcodx * heightmap_size;
  vec2 dtcdy = dcody * heightmap_size;
  float lod = max(0.0, 0.5 * log2(min(dot(dtcdx, dtcdx), dot(dtcdy, dtcdy))));

  vec3 step_W = dWdx * ss.x + dWdy * ss.y;
  vec3 W = WorldPos + e * step_W;

  /* linear search */
  for (float i = 0.0; i < samples_count; i++) {
    vec3 next = uvw + step;
    vec3 next_W = W + step_W;

    float h = textureLod(heightmap, next.xy, lod).x;
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
    float h = textureLod(heightmap, next.xy, lod).x;
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
  float dHdu = textureLod(heightmap, uvw.xy + vec2(radius, 0.0), lod).x -
               textureLod(heightmap, uvw.xy - vec2(radius, 0.0), lod).x;
  float dHdv = textureLod(heightmap, uvw.xy + vec2(0.0, radius), lod).x -
               textureLod(heightmap, uvw.xy - vec2(0.0, radius), lod).x;

  vec3 dPdu = f * (dcody.y * dPdx - dcodx.y * dPdy);
  vec3 dPdv = f * (dcodx.x * dPdy - dcody.x * dPdx);
  vec3 r1 = cross(dPdv, N);
  vec3 r2 = cross(N, dPdu);

  float dr = dot(dPdu, r1) * radius * 2.0;
  vec3 surfgrad = dHdu * r1 + dHdv * r2;
  normal = mat3(inverse(view)) * normalize(abs(dr) * N - (sign(dr) * scale) * surfgrad);

  if (projection[3][3] == 0.0 && NdotI > -vz)
    discard;
}

// ----------------------------------------------------------------------------

void main()
{
    vec2 pTexCoords = TexCoords;
    vec3 pNormal = Normal;
    vec3 pWorldPos = WorldPos;

    if (material.heightMap) {
        vec3 out_co;
        float depth_offset;

        // TODO: variable
        node_parallax_occlusion_map(
            vec3(TexCoords, 0.0),
            material.parallaxMapMidLevel,
            material.parallaxMapScale,
            material.parallaxMapSampleCount,
            material.parallaxMapScaleMode,
            ViewPos,
            ViewNormal,
            texture_height1,
            out_co,                 // out vec3 out_co,
            depth_offset,           // out float depth,
            pNormal,                // out vec3 normal
            pWorldPos               // out vec3 out_worldposition
        );

        pTexCoords = out_co.xy;
        
        // TODO: disable option
        // TODO: use depth_offset
        vec4 v_clip_coord = (projection * view * vec4(pWorldPos, 1));
        float f_ndc_depth = v_clip_coord.z / v_clip_coord.w;
        gl_FragDepth = (1.0 - 0.0) * 0.5 * f_ndc_depth + (1.0 + 0.0) * 0.5;
    } else {
        // TODO: when exactly should set?
        gl_FragDepth = gl_FragCoord.z;
    }

    // TODO: preprocessor texture read - material properties
    vec3 albedo;
    float ao, roughness, metallic;

    if (material.albedoMap) {
        albedo = texture(texture_diffuse1, pTexCoords).rgb;
        // TODO: validate
        albedo *= material.albedo.rgb;
    } else {
        albedo = material.albedo.rgb;
    }

    vec3 emissive = material.emissiveColor.xyz * material.emissiveStrength;
    albedo += emissive;

    // TODO: merge detection
    if (material.aoRoughMetalMap) {
        // TODO: individual present
        vec3 merged = texture(texture_unknown1, pTexCoords).rgb;
        ao = 1.0; // TODO: useAOMAP
        roughness = merged.g;
        metallic = merged.b;
    } else {
        if (material.aoMap) {
            ao = texture(texture_ao1, pTexCoords).r;
        } else {
            ao = 1.0;
        }
        if (material.roughMap) {
            roughness = texture(texture_rough1, pTexCoords).r;
        } else {
            roughness = material.roughness;
        }
        if (material.metalMap) {
            metallic = texture(texture_metal1, pTexCoords).r;
        } else {
            metallic = material.metallic;
        }
    }

    // TODO: pom view space position and normal?
    vec3 N;
    vec3 ViewN;
    if (material.normalMap) {
        vec3 tangentNormal = texture(texture_normal1, pTexCoords).xyz * 2.0 - 1.0;
        // TODO: pom tangent and bitangent?
        mat3 TBN = mat3(
            Tangent,
            Bitangent,
            pNormal
        );
        N = normalize(TBN * tangentNormal);
        ViewN = normalize(ViewTBN * tangentNormal);
    } else {
        N = normalize(pNormal);
        ViewN = normalize(ViewNormal);
    }

    gPosition = pWorldPos;
    gNormalShadow = vec4(N, getVisibility(pWorldPos, pNormal));
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