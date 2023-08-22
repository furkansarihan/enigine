#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix;
layout (location = 7) in vec3 alightColor;
layout (location = 8) in float aRadius;
layout (location = 9) in float aLinear;
layout (location = 10) in float aQuadratic;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 position;
out vec3 color;
out float radius;
out float linear;
out float quadratic;

void main()
{
    TexCoords = aTexCoords;
    // gl_Position = projection * view * model * vec4(aPos + aInstancePos, 1.0);
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);

    position = aInstanceMatrix[3].xyz;
    color = alightColor;
    radius = aRadius;
    linear = aLinear;
    quadratic = aQuadratic;
}
