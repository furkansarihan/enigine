#version 410 core

layout (location = 0) in vec2 gridPos;

uniform mat4 worldViewProjMatrix;
uniform mat4 M;
uniform mat4 V;
uniform float minHeight;
uniform float maxHeight;
uniform float scaleHoriz;
uniform vec4 scaleFactor;
uniform vec4 fineTextureBlockOrigin; 
uniform vec3 viewerPos;
uniform vec2 terrainSize;
uniform vec2 uvOffset;

// shadowmap
uniform vec3 lightDirection;

uniform sampler2D elevationSampler;

out float _height; // based on world coorinates
out vec2 _tuv; // texture coordinates
out vec2 _zalpha; // coordinates for elevation-map lookup
out float _distance; // vertex distance to the camera
out vec3 _normal; // vertex normal

// shadowmap
out vec3 Position_worldspace;
out vec3 LightDirection_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 Normal_cameraspace;

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
    float yScaleFactor = maxHeight - minHeight;

    // convert from grid xy to world xy coordinates
    //  scaleFactor.xy: grid spacing of current level // scale
    //  scaleFactor.zw: origin of current block within world // translate
    vec2 pos = (M * vec4(gridPos.x, 1, gridPos.y, 1)).xz;
    vec2 worldPos = pos * scaleFactor.xy + scaleFactor.zw;

    // compute coordinates for vertex texture
    //  FineBlockOrig.xy: 1/(w, h) of texture // normalized size
    //  FineBlockOrig.zw: origin of block in texture // translate       
    // vec2 uv = vec2(gridPos * fineTextureBlockOrigin.xy + fineTextureBlockOrigin.zw) + uvOffset;
    // vec2 uv = gridPos * fineTextureBlockOrigin.xy + fineTextureBlockOrigin.zw;
    vec2 uv = worldPos * fineTextureBlockOrigin.xy + uvOffset;

    uv /= scaleHoriz;

    float height = texture(elevationSampler, uv).r;

    height *= yScaleFactor;
    height += minHeight;

    vec3 position_worldspace = vec3(worldPos.x, height, worldPos.y);

    gl_Position = worldViewProjMatrix * vec4(position_worldspace, 1);

    float x = uv.x * terrainSize.x;
    float y = uv.y * terrainSize.y;

    _height = height;
    _tuv = vec2(x, y);
    _distance = clamp(abs(distance(position_worldspace, viewerPos)), 0, 10000);

    Position_worldspace = position_worldspace;

    vec3 vertexPosition_cameraspace = (V * vec4(lightDirection, 0)).xyz;
    EyeDirection_cameraspace = vec3(0, 0, 0) - vertexPosition_cameraspace;
    
    vec3 LightPosition_cameraspace = vec3(0, 0, 0) - (V * vec4(position_worldspace, 1)).xyz;
    LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

    // compute normal
    float texelSize = fineTextureBlockOrigin.x * scaleFactor.x;
    float scale = scaleFactor.x;
    float texelAspect = yScaleFactor;

    _normal = computeNormal(uv, texelSize, yScaleFactor, scale);
}
