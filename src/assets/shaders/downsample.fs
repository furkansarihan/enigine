#version 410 core

// This shader performs downsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.
// This particular method was customly designed to eliminate
// "pulsating artifacts and temporal stability issues".

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!
uniform sampler2D srcTexture;
uniform vec2 srcResolution;

// which mip we are writing to, used for Karis average
uniform int mipLevel = 1;

uniform float threshold;
uniform float softThreshold;

in vec2 texCoord;
layout (location = 0) out vec3 downsample;

vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float invGamma = 1.0 / 2.2;
vec3 ToSRGB(vec3 v)   { return PowVec3(v, invGamma); }

float sRGBToLuma(vec3 col)
{
    return dot(col, vec3(0.2126, 0.7152, 0.0722));
    // return dot(col, vec3(0.299, 0.587, 0.114));
}

float KarisAverage(vec3 col)
{
    // Formula is 1 / (1 + luma)
    float luma = sRGBToLuma(ToSRGB(col));
    return 1.0 / (1.0 + luma);
}

// vec3 prefilter(vec3 c) {
//     float brightness = max(c.r, max(c.g, c.b));
//     float knee = threshold * softThreshold;
//     float soft = brightness - threshold + knee;
//     soft = clamp(soft, 0, 2 * knee);
//     soft = soft * soft / (4 * knee + 0.00001);
//     float contribution = max(soft, brightness - threshold);
//     contribution /= max(brightness, 0.00001);
//     return c * contribution;
// }

void main()
{
    vec2 srcTexelSize = 1.0 / srcResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(srcTexture, vec2(texCoord.x - 2*x, texCoord.y + 2*y)).rgb;
    vec3 b = texture(srcTexture, vec2(texCoord.x,       texCoord.y + 2*y)).rgb;
    vec3 c = texture(srcTexture, vec2(texCoord.x + 2*x, texCoord.y + 2*y)).rgb;

    vec3 d = texture(srcTexture, vec2(texCoord.x - 2*x, texCoord.y)).rgb;
    vec3 e = texture(srcTexture, vec2(texCoord.x,       texCoord.y)).rgb;
    vec3 f = texture(srcTexture, vec2(texCoord.x + 2*x, texCoord.y)).rgb;

    vec3 g = texture(srcTexture, vec2(texCoord.x - 2*x, texCoord.y - 2*y)).rgb;
    vec3 h = texture(srcTexture, vec2(texCoord.x,       texCoord.y - 2*y)).rgb;
    vec3 i = texture(srcTexture, vec2(texCoord.x + 2*x, texCoord.y - 2*y)).rgb;

    vec3 j = texture(srcTexture, vec2(texCoord.x - x, texCoord.y + y)).rgb;
    vec3 k = texture(srcTexture, vec2(texCoord.x + x, texCoord.y + y)).rgb;
    vec3 l = texture(srcTexture, vec2(texCoord.x - x, texCoord.y - y)).rgb;
    vec3 m = texture(srcTexture, vec2(texCoord.x + x, texCoord.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1

    // Check if we need to perform Karis average on each block of 4 samples
    vec3 groups[5];
    switch (mipLevel)
    {
    case 0:
      // We are writing to mip 0, so we need to apply Karis average to each block
      // of 4 samples to prevent fireflies (very bright subpixels, leads to pulsating
      // artifacts).
      groups[0] = (a+b+d+e) * 0.125 / 4.0;
      groups[1] = (b+c+e+f) * 0.125 / 4.0;
      groups[2] = (d+e+g+h) * 0.125 / 4.0;
      groups[3] = (e+f+h+i) * 0.125 / 4.0;
      groups[4] = (j+k+l+m) * 0.500 / 4.0;
      float kw0 = KarisAverage(groups[0]);
      float kw1 = KarisAverage(groups[1]);
      float kw2 = KarisAverage(groups[2]); 
      float kw3 = KarisAverage(groups[3]);
      float kw4 = KarisAverage(groups[4]);
      downsample = (kw0*groups[0] + kw1*groups[1] + kw2*groups[2] + kw3*groups[3] + kw4*groups[4]) 
                    / (kw0+kw1+kw2+kw3+kw4);
      downsample = max(downsample, 0.0001);
      break;
    default:
      downsample = e*0.125;
      downsample += (a+c+g+i)*0.03125;
      downsample += (b+d+f+h)*0.0625;
      downsample += (j+k+l+m)*0.125;
      break;
    }
}
