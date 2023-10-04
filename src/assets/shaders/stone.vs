#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 WorldPos;
out vec3 Normal;
out vec2 TexCoords;
out float _height;
out vec3 _normal;

out vec3 ViewPos;
out vec3 ViewNormal;

uniform sampler2D elevationSampler;

uniform vec2 elevationMapSize;
uniform float minHeight;
uniform float maxHeight;
uniform float scaleHoriz;
uniform float u_time;
uniform float mult;
uniform vec2 worldOrigin;

uniform mat4 projection;
uniform mat4 view;

uniform vec2 referencePos;
uniform int columnCount;

float rand(float st) {
    return fract(sin(dot(vec2(st), vec2(12.9898,78.233))) * 43758.5453123);
}

// 2D Random
float random (in vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

vec3 computeNormal(vec2 uv, float texelSize, float texelAspect, float scale)
{
    vec4 h;
    h[0] = texture(elevationSampler, uv + texelSize * vec2(0, -1)).r * texelAspect;
    h[1] = texture(elevationSampler, uv + texelSize * vec2(-1, 0)).r * texelAspect;
    h[2] = texture(elevationSampler, uv + texelSize * vec2(1, 0)).r * texelAspect;
    h[3] = texture(elevationSampler, uv + texelSize * vec2(0, 1)).r * texelAspect;
    vec3 n;
    n.z = h[0] - h[3];
    n.x = h[1] - h[2];
    n.y = 2 * scale;

    return normalize(n);
}

void main()
{
    TexCoords = aTexCoords;

    float halfTileSize = mult / 2;

    float x = gl_InstanceID / columnCount;
    float z = mod(gl_InstanceID, columnCount);

    x *= mult;
    z *= mult;

    // move to center
    x += halfTileSize;
    z += halfTileSize;

    vec2 worldPos = referencePos + vec2(x, z);

    float n = noise(worldPos);
    float nx = clamp(n * 12, 0, halfTileSize);
    float nz = clamp(n * 8, 0, halfTileSize);

    worldPos.x += nx;
    worldPos.y -= nz;

    vec2 uv = worldPos * elevationMapSize;
    uv /= scaleHoriz;

    float height = texture(elevationSampler, uv).r;
    _height = height;

    float yScaleFactor = maxHeight - minHeight;
    height *= yScaleFactor;
    height += minHeight;

    vec3 localPos = aPos * vec3(0.2);

    WorldPos = vec3(worldPos.x, height, worldPos.y) + localPos;
    WorldPos += vec3(worldOrigin.x, 0, worldOrigin.y);
    // TODO: ?
    Normal = aNormal * -1.0;

    ViewPos = (view * vec4(WorldPos, 1.0)).xyz;
    // TODO?
    ViewNormal = mat3(transpose(inverse(view))) * aNormal * -1;

    gl_Position = projection * view * vec4(WorldPos, 1);

    // normal
    float texelSize = elevationMapSize.x;
    float scale = 1;
    float texelAspect = yScaleFactor;

    _normal = computeNormal(uv, texelSize, texelAspect, scale);
    float slope = dot(vec3(0, 1, 0), _normal);

    // TODO: better culling
    float heightPercent = height / yScaleFactor;

    if (slope < 0.6 || heightPercent < 0.004) {
        gl_Position = vec4(0, 0, 0, -1);
    }
}