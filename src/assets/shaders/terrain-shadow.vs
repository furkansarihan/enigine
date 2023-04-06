#version 410 core

layout (location = 0) in vec2 gridPos;

uniform mat4 worldViewProjMatrix;
uniform mat4 M;
uniform float minHeight;
uniform float maxHeight;
uniform float scaleHoriz;
uniform vec4 scaleFactor;
uniform vec4 fineTextureBlockOrigin;
uniform vec3 lightPos;

uniform vec2 uvOffset;

uniform sampler2D elevationSampler;

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
}
