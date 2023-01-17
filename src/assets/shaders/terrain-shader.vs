#version 410 core

layout (location = 0) in vec2 gridPos;

uniform mat4 WorldViewProjMatrix;
uniform float ZScaleFactor;
uniform vec4 ScaleFactor;
uniform vec3 RotateFactor;
uniform vec4 FineTextureBlockOrigin; 
uniform vec2 AlphaOffset;
uniform vec3 ViewerPos;
// transition width (chosen to be n/10) ? [0, n-1] : range of clipmap grid
uniform float OneOverWidth; 

uniform vec2 UvOffset;

uniform usampler2D ElevationSampler;

out vec2 _uv; // coordinates for normal-map lookup
out vec2 _zalpha; // coordinates for elevation-map lookup
out float _distance; // vertex distance to the camera

void main()
{
    // convert from grid xy to world xy coordinates
    //  ScaleFactor.xy: grid spacing of current level // scale
    //  ScaleFactor.zw: origin of current block within world // translate
    vec2 worldPos = gridPos * ScaleFactor.xy + ScaleFactor.zw;

    // compute coordinates for vertex texture
    //  FineBlockOrig.xy: 1/(w, h) of texture // normalized size
    //  FineBlockOrig.zw: origin of block in texture // translate       
    // vec2 uv = vec2(gridPos * FineTextureBlockOrigin.xy + FineTextureBlockOrigin.zw) + UvOffset;
    // vec2 uv = gridPos * FineTextureBlockOrigin.xy + FineTextureBlockOrigin.zw;
    vec2 uv = worldPos * FineTextureBlockOrigin.xy + UvOffset;

    // https://www.khronos.org/opengl/wiki/Sampler_(GLSL)#Texture_lookup_functions
    // sample the vertex texture
    // float zf_zd = texture(ElevationSampler, vec4(uv, 0, 1));
    float zf_zd = texture(ElevationSampler, uv).r;

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
    vec2 alpha = clamp((abs(worldPos - ViewerPos.xz) - AlphaOffset) * OneOverWidth, 0, 1);
    alpha.x = max(alpha.x, alpha.y);

    float z = zf + alpha.x * zd;
    z = z * ZScaleFactor;

    gl_Position = WorldViewProjMatrix * vec4(worldPos.x, zf_zd * ZScaleFactor, worldPos.y, 1);

    _uv = uv;
    _zalpha = vec2(0.5 + z/1600, alpha.x);
    _distance = clamp(abs(distance(vec3(worldPos.x, zf_zd * ZScaleFactor, worldPos.y), ViewerPos)), 0, 10000);
}
