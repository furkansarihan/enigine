#version 410 core

layout (location = 0) in vec3 vertexPosition_modelspace;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexUV;

layout (location = 3) in vec3 position;
layout (location = 4) in vec3 emitPosition;
layout (location = 5) in vec3 velocity;
layout (location = 6) in float duration;
layout (location = 7) in float maxDuration;
layout (location = 8) in float distance;

uniform mat4 u_viewProjection;
uniform vec3 u_worldOrigin;
uniform vec3 u_viewPosition;
uniform float u_particleScale;

out vec2 UV;

const float PI_2 = 1.5707;

mat4 translate(mat4 mat, vec3 translation) {
    mat[3] += vec4(translation, 0.0);
    return mat;
}

mat4 rotate(mat4 mat, float angle, vec3 axis) {
    float c = cos(angle);
    float s = sin(angle);
    vec3 temp = (1.0 - c) * axis;

    mat4 rotation = mat4(
        c + temp.x * axis.x, temp.x * axis.y - s * axis.z, temp.x * axis.z + s * axis.y, 0.0,
        temp.y * axis.x + s * axis.z, c + temp.y * axis.y, temp.y * axis.z - s * axis.x, 0.0,
        temp.z * axis.x - s * axis.y, temp.z * axis.y + s * axis.x, c + temp.z * axis.z, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    return mat * rotation;
}

mat4 scale(mat4 mat, vec3 scaling) {
    mat[0] *= scaling.x;
    mat[1] *= scaling.y;
    mat[2] *= scaling.z;
    return mat;
}

// TODO: vertex attribute?
mat4 getModelMatrix() {
    mat4 model = mat4(1.0);
    // Translate
    model = translate(model, position + u_worldOrigin);
    // Bilboarding
    vec3 camToParticle = normalize(position - u_viewPosition);
    vec3 rotation = vec3(0.0);
    rotation.y = -atan(camToParticle.x, camToParticle.z) + PI_2;
    rotation.z = -atan(camToParticle.y, length(camToParticle.xz));
    model = rotate(model, rotation.y, vec3(0, 1, 0));
    model = rotate(model, rotation.z, vec3(0, 0, 1));
    // Scale
    model = scale(model, vec3(u_particleScale));
    return model;
}

void main()
{
    mat4 MVP = u_viewProjection * getModelMatrix();

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
