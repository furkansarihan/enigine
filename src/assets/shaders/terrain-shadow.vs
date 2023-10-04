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
uniform vec2 worldOrigin;

uniform sampler2D elevationSampler;

void main()
{
    float yScaleFactor = maxHeight - minHeight;

    vec2 pos = (M * vec4(gridPos.x, 1, gridPos.y, 1)).xz;
    vec2 worldPos = pos * scaleFactor.xy + scaleFactor.zw;

    vec2 uv = worldPos * fineTextureBlockOrigin.xy;

    uv /= scaleHoriz;

    float height = texture(elevationSampler, uv).r;

    height *= yScaleFactor;
    height += minHeight;

    vec3 position_worldspace = vec3(worldPos.x, height, worldPos.y);
    position_worldspace += vec3(worldOrigin.x, 0, worldOrigin.y);

    gl_Position = worldViewProjMatrix * vec4(position_worldspace, 1);
}
