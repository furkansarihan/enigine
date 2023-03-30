#version 410 core

uniform sampler2D renderedTexture;
uniform vec2 screenSize;

// blur
uniform float blurOffset;

// hdr
uniform float exposure;

in vec2 UV;

out vec4 fragColor;

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

vec3 hdr(vec3 color, float exposure)
{
    const float gamma = 2.2;

    // reinhard tone mapping
    // vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-color * exposure);

    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));

    return mapped;
}

void main(void)
{
    // fragColor.xyz = texture(renderedTexture, UV).xyz;
    // return;

    // fragColor.xyz = blur(renderedTexture, UV);
    fragColor.xyz = fxaa(renderedTexture, UV);
    
    // HDR tone mapping - gamma correction
    fragColor.xyz = hdr(fragColor.xyz, exposure);

    fragColor.a = 1.0;
}
