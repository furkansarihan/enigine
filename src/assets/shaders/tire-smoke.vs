#version 410 core

layout (location = 0) in vec3 vertexPosition_modelspace;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexUV;

uniform mat4 MVP;
uniform float duration;
uniform float maxDuration;

out vec2 UV;

void main()
{
    // TODO: particle engine integration
    int rowSize = 8;
    int columnSize = 8;
    int tileCount = 63; // row * column - 1

    // TODO: adapt to duration
    int skipTile = 32;
    tileCount -= skipTile;

    float tileIndexRate = clamp(1 - duration / maxDuration, 0, 1);
    float tileIndex = round(tileIndexRate * tileCount);
    
    tileIndex += skipTile;

    int rowIndex = int(tileIndex / rowSize);
    int columnIndex = int(tileIndex) % rowSize;

    // TODO: check reverse row
    // rowIndex = rowSize - rowIndex;

    vec2 tileUVSize = vec2(1, 1) / vec2(columnSize, rowSize);

    UV = tileUVSize * vec2(columnIndex, rowIndex);
    UV += vertexUV * tileUVSize;

    gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);
}
