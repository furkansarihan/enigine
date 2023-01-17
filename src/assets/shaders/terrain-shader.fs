#version 410 core

uniform vec3 LightDirection;
uniform bool wireframe;
uniform vec3 WireColor;

uniform sampler2D NormalMapSampler;
// uniform sampler2D ZBasedColorSampler;

in vec2 _uv; // coordinates for normal-map lookup
in vec2 _zalpha; // coordinates for elevation-map lookup
in float _distance; // vertex distance to the camera

out vec4 color;

void main()
{
    // do a texture lookup to get the normal in current level
    vec4 normalfc = texture(NormalMapSampler, _uv);
    // normal_fc.xy contains normal at current (fine) level
    // normal_fc.zw contains normal at coarser level
    // blend normals using alpha computed in vertex shader  
    vec3 normal = vec3((1 - _zalpha.y) * normalfc.xy + _zalpha.y * (normalfc.zw), 1.0);

    // unpack coordinates from [0, 1] to [-1, +1] range, and renormalize.
    normal = normalize(normalfc.xyz * 2 - 1);

    float s = clamp(dot(normal, LightDirection), 0.5, 1);

    if (wireframe) {
        float distanceRate = (_distance / 10000);
        float normalRate = 1 - distanceRate;
        color = vec4(WireColor, 1 * normalRate);
    } else {
        // color = vec4(texture(ZBasedColorSampler, _uv).rgb * s, 1);
        color = vec4(vec3(0.6, 0.6, 0.6).rgb * s, 1);
    }
}
