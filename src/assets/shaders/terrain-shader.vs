#version 410 core

layout (location = 0) in vec2 gridPos;

uniform mat4 worldViewProjMatrix;
uniform float zscaleFactor;
uniform vec4 scaleFactor;
uniform vec4 fineTextureBlockOrigin; 
uniform vec2 alphaOffset;
uniform vec3 viewerPos;
// transition width (chosen to be n/10) ? [0, n-1] : range of clipmap grid
uniform float oneOverWidth; 
uniform vec2 terrainSize;

uniform vec2 uvOffset;

uniform usampler2D elevationSampler;

out float _height; // based on world coorinates
out vec2 _tuv; // texture coordinates
out vec2 _uv; // coordinates for normal-map lookup
out vec2 _zalpha; // coordinates for elevation-map lookup
out float _distance; // vertex distance to the camera

void main()
{
    // convert from grid xy to world xy coordinates
    //  scaleFactor.xy: grid spacing of current level // scale
    //  scaleFactor.zw: origin of current block within world // translate
    vec2 worldPos = gridPos * scaleFactor.xy + scaleFactor.zw;

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

    gl_Position = worldViewProjMatrix * vec4(worldPos.x, zf_zd * zscaleFactor, worldPos.y, 1);

    float x = uv.x * terrainSize.x;
    float y = uv.y * terrainSize.y;

    _height = zf_zd * zscaleFactor;
    _tuv = vec2(x, y);
    _uv = uv;
    _zalpha = vec2(0.5 + z/1600, alpha.x);
    _distance = clamp(abs(distance(vec3(worldPos.x, zf_zd * zscaleFactor, worldPos.y), viewerPos)), 0, 10000);
}
