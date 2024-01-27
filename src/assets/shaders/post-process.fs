#version 410 core

uniform sampler2D renderedTexture;
uniform sampler2D bloomTexture;
uniform vec2 screenSize;

// blur
uniform float blurOffset;

// hdr
uniform float exposure;

// effect
uniform float contrastBright;
uniform float contrastDark;
uniform float bloomIntensity;
uniform float grainAmount;

// tone mapping
uniform float u_A;
uniform float u_B;
uniform float u_C;
uniform float u_D;
uniform float u_E;
uniform float u_F;
uniform float u_W;
uniform float u_exposure;
uniform float u_gamma;

in vec2 UV;

out vec3 fragColor;

vec3 blur(sampler2D tex, vec2 uv) {
    float offset = blurOffset;

    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );

    float kernel[9] = float[](
          1 / 16.0, 2 / 16.0, 1 / 16.0,
          2 / 16.0, 4 / 16.0, 2 / 16.0,
          1 / 16.0, 2 / 16.0, 1 / 16.0
    );
    
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = texture(tex, uv.st + offsets[i]).xyz;
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];
    
    return col;
}

vec3 fxaa(sampler2D tex, vec2 uv) {
    float FXAA_SPAN_MAX = 8.0;
    float FXAA_REDUCE_MUL = 1.0 / 8.0;
    float FXAA_REDUCE_MIN = 1.0 / 128.0;

    vec3 rgbNW = texture(tex, uv + (vec2(-1.0,-1.0) / screenSize)).xyz;
    vec3 rgbNE = texture(tex, uv + (vec2(1.0,-1.0) / screenSize)).xyz;
    vec3 rgbSW = texture(tex, uv + (vec2(-1.0,1.0) / screenSize)).xyz;
    vec3 rgbSE = texture(tex, uv + (vec2(1.0,1.0) / screenSize)).xyz;
    vec3 rgbM = texture(tex, uv).xyz;

    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) / screenSize;

    vec3 rgbA = (1.0 / 2.0) * (
        texture(tex, uv.xy + dir * (1.0 / 3.0 - 0.5)).xyz +
        texture(tex, uv.xy + dir * (2.0 / 3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * (1.0 / 2.0) + (1.0 / 4.0) * (
        texture(tex, uv.xy + dir * (0.0 / 3.0 - 0.5)).xyz +
        texture(tex, uv.xy + dir * (3.0 / 3.0 - 0.5)).xyz);
    float lumaB = dot(rgbB, luma);

    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        return rgbA;
    } else {
        return rgbB;
    }
}

#include <tonemapping.fs>

void main(void)
{
    // fragColor = blur(renderedTexture, UV);
    fragColor = fxaa(renderedTexture, UV);

    // exposure
    fragColor *= pow(2.0, u_exposure);

    float brightness = dot(fragColor, vec3(0.2126, 0.7152, 0.0722));
    float darkness = 1 - brightness;
    if (darkness < 0) darkness = 0;

    fragColor += fragColor * brightness * brightness * contrastBright;
    fragColor -= fragColor * darkness * darkness * contrastDark;

    vec3 bloomColor = texture(bloomTexture, UV).rgb;
    // fragColor += bloomColor.xyz * bloomIntensity;
    fragColor = mix(fragColor, bloomColor.xyz, bloomIntensity);

    // tone mapping
    fragColor = Tonemap_Filmic_UC2(fragColor, u_W, u_A, u_B, u_C, u_D, u_E, u_F);

    // gamma correction 
    fragColor = pow(fragColor, vec3(1.0 / u_gamma));

    // debug
    // fragColor = bloomColor.xyz;
    // fragColor = bloomColor.xyz * bloomIntensity;
    // fragColor = mix(vec3(0, 0, 0), bloomColor.xyz, bloomIntensity);
}
