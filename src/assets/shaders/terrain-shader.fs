#version 410 core

uniform vec3 lightDirection;
uniform bool wireframe;
uniform vec3 wireColor;
uniform float fogMaxDist;
uniform float fogMinDist;
uniform vec4 fogColor;

uniform sampler2D normalMapSampler;
uniform sampler2DArray textureSampler;

in float _height; // based on world coorinates
in vec2 _tuv; // texture coordinates
in vec2 _uv; // coordinates for normal-map lookup
in vec2 _zalpha; // coordinates for elevation-map lookup
in float _distance; // vertex distance to the camera

out vec4 color;

vec3 transitionRegion(int from, int to, float regionEnd, float regionSize) {
    vec3 color1 = texture(textureSampler, vec3(_tuv, from)).rgb;
    vec3 color2 = texture(textureSampler, vec3(_tuv, to)).rgb;

    float dist = regionEnd - _height;
    float mixFactor = dist / regionSize;

    return mix(color2, color1, mixFactor);
}

void main()
{
    // do a texture lookup to get the normal in current level
    vec4 normalfc = texture(normalMapSampler, _uv);
    // normal_fc.xy contains normal at current (fine) level
    // normal_fc.zw contains normal at coarser level
    // blend normals using alpha computed in vertex shader  
    vec3 normal = vec3((1 - _zalpha.y) * normalfc.xy + _zalpha.y * (normalfc.zw), 1.0);

    // unpack coordinates from [0, 1] to [-1, +1] range, and renormalize.
    normal = normalize(normalfc.xyz * 2 - 1);

    float s = clamp(dot(normal, lightDirection), 0.5, 1);

    vec4 outColor;
    if (wireframe) {
        float distanceRate = (_distance / 10000);
        float normalRate = 1 - distanceRate;
        outColor = vec4(wireColor, 1 * normalRate);
    } else {
        // outColor = vec4(texture(textureSampler, _tuv).rgb * s, 1); // sampler2D
        if (_height < 4) {
            int index = 0; // water
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb * s, 1); // sampler2DArray
        } else if (_height >= 4 && _height < 8) {
            vec3 mergedColor = transitionRegion(0, 1, 8, 4);
            outColor = vec4(mergedColor * s, 1);
        } else if (_height < 12) {
            int index = 1; // sand
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb * s, 1);
        } else if (_height >= 12 && _height < 16) {
            vec3 mergedColor = transitionRegion(1, 2, 16, 4);
            outColor = vec4(mergedColor * s, 1);
        } else if (_height < 20) {
            int index = 2; // stone
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb * s, 1);
        } else if (_height >= 20 && _height < 24) {
            vec3 mergedColor = transitionRegion(2, 3, 24, 4);
            outColor = vec4(mergedColor * s, 1);
        } else if (_height < 50) {
            int index = 3; // grass
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb * s, 1);
        } else if (_height >= 50 && _height < 54) {
            vec3 mergedColor = transitionRegion(3, 4, 54, 4);
            outColor = vec4(mergedColor * s, 1);
        } else if (_height < 160) {
            int index = 4; // rock
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb * s, 1);
        } else if (_height >= 160 && _height < 164) {
            vec3 mergedColor = transitionRegion(4, 5, 164, 4);
            outColor = vec4(mergedColor * s, 1);
        } else {
            int index = 5; // snow
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb * s, 1);
        }
    }

    // fog
    float fogFactor = (fogMaxDist - _distance) / (fogMaxDist - fogMinDist);
    fogFactor = clamp(fogFactor, 0, 1);
    color = mix(fogColor, outColor, fogFactor);
}
