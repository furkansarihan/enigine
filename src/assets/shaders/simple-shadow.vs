#version 410 core

layout (location = 0) in vec3 vertexPosition_modelspace;
layout (location = 1) in vec2 vertexUV;
layout (location = 2) in vec3 vertexNormal_modelspace;

uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 LightInvDirection_worldspace;

out vec3 VertexPosition_modelspace;
out vec3 Position_worldspace;
out vec3 LightDirection_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 Normal_cameraspace;

void main()
{
    VertexPosition_modelspace = vertexPosition_modelspace;

    // Output position of the vertex, in clip space : MVP * position
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1);

    // Position of the vertex, in worldspace : M * position
    Position_worldspace = (M * vec4(vertexPosition_modelspace, 1)).xyz;

    // Vector that goes from the vertex to the light, in camera space
    LightDirection_cameraspace = (V * vec4(LightInvDirection_worldspace, 0)).xyz;

    // Vector that goes from the vertex to the camera, in camera space.
    // In camera space, the camera is at the origin (0,0,0).
    EyeDirection_cameraspace = vec3(0,0,0) - ( V * M * vec4(vertexPosition_modelspace, 1)).xyz;

    // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
    Normal_cameraspace = (V * M * vec4(vertexNormal_modelspace, 0)).xyz; 
}
