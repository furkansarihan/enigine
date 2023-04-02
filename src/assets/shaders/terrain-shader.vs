#version 410 core

layout (location = 0) in vec2 gridPos;

uniform mat4 worldViewProjMatrix;
uniform mat4 M;
uniform mat4 V;
uniform float zscaleFactor;
uniform vec4 scaleFactor;
uniform vec4 fineTextureBlockOrigin; 
uniform vec2 alphaOffset;
uniform vec3 viewerPos;
// transition width (chosen to be n/10) ? [0, n-1] : range of clipmap grid
uniform float oneOverWidth; 
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

    // https://www.khronos.org/opengl/wiki/Sampler_(GLSL)#Texture_lookup_functions
    // sample the vertex texture
    // float zf_zd = texture(elevationSampler, vec4(uv, 0, 1));
    float zf_zd = texture(elevationSampler, uv).r;

    // TODO: texture format
    // normalize
    // zf_zd = zf_zd / 255;

    // unpack to obtain zf and zd = (zc - zf)
    //  zf is elevation value in current (fine) level
    //  zc is elevation value in coarser level
    float zf = floor(zf_zd);
    // zd IN [-256, 256], fract(zf_zd) IN [0, 1]
    float zd = fract(zf_zd) * 512 - 256;       // zd = zc - z

    // compute alpha (transition parameter), and blend elevation.
    //
    // info:
    // The desired property is that evaluates to 0 
    // except in the transition region, 
    // where it ramps up linearly to reach 1 at the outer perimeter.
    vec2 alpha = clamp((abs(worldPos - viewerPos.xz) - alphaOffset) * oneOverWidth, 0, 1);
    alpha.x = max(alpha.x, alpha.y);

    float z = zf + alpha.x * zd;
    z = z * zscaleFactor;

    vec3 position_worldspace = vec3(worldPos.x, zf_zd * zscaleFactor, worldPos.y);

    gl_Position = worldViewProjMatrix * vec4(position_worldspace, 1);

    float x = uv.x * terrainSize.x;
    float y = uv.y * terrainSize.y;

    _height = zf_zd * zscaleFactor;
    _tuv = vec2(x, y);
    _zalpha = vec2(0.5 + z/1600, alpha.x);
    _distance = clamp(abs(distance(position_worldspace, viewerPos)), 0, 10000);

    Position_worldspace = position_worldspace;

    vec3 vertexPosition_cameraspace = (V * vec4(lightDirection, 0)).xyz;
    EyeDirection_cameraspace = vec3(0, 0, 0) - vertexPosition_cameraspace;
    
    vec3 LightPosition_cameraspace = vec3(0, 0, 0) - (V * vec4(position_worldspace, 1)).xyz;
    LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

    // compute normal
    float texelSize = fineTextureBlockOrigin.x * scaleFactor.x;
    float scale = scaleFactor.x;
    float texelAspect = zscaleFactor;

    _normal = computeNormal(uv, texelSize, zscaleFactor, scale);
}
