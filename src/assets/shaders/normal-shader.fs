#version 410 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_worldspace;

in vec3 LightDirection_tangentspace;
in vec3 EyeDirection_tangentspace;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
// uniform vec3 MaterialDiffuseColor;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

// Light source
uniform vec3 LightPosition_worldspace;
uniform float LightPower;
uniform vec3 LightColor;

void main()
{        
    vec3 MaterialDiffuseColor = texture(texture_diffuse1, UV).rgb;
    vec3 MaterialAmbientColor = vec3(0.2, 0.2, 0.2) * MaterialDiffuseColor;
    vec3 MaterialSpecularColor = vec3(0.3, 0.3, 0.3);

    // Local normal, in tangent space. V tex coordinate is inverted because normal map is in TGA (not in DDS) for better quality
    vec3 TextureNormal_tangentspace = normalize(texture(texture_normal1, UV).rgb * 2.0 - 1.0);

    // Distance to the light
    float distance = distance(Position_worldspace, LightPosition_worldspace);

    // Normal of the computed fragment, in camera space
    vec3 n = TextureNormal_tangentspace;
    // Direction of the light (from the fragment to the light)
    vec3 l = normalize(LightDirection_tangentspace);
    // Cosine of the angle between the normal and the light direction, 
    // clamped above 0
    //  - light is at the vertical of the triangle -> 1
    //  - light is perpendicular to the triangle -> 0
    //  - light is behind the triangle -> 0
    float cosTheta = clamp(dot(n, l), 0, 1);

    // Eye vector (towards the camera)
    vec3 E = normalize(EyeDirection_tangentspace);
    // Direction in which the triangle reflects the light
    vec3 R = reflect(-l, n);
    // Cosine of the angle between the Eye vector and the Reflect vector,
    // clamped to 0
    //  - Looking into the reflection -> 1
    //  - Looking elsewhere -> < 1
    float cosAlpha = clamp(dot(E, R), 0, 1);
    
    // Output color = color of the texture at the specified UV
    color.rgb = 
        // Ambient : simulates indirect lighting
        MaterialAmbientColor +
        // Diffuse : "color" of the object
        MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance * distance) +
        // Specular : reflective highlight, like a mirror
        MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha, 5) / (distance * distance);
    // color.xyz = LightDirection_tangentspace;
    color.a = 1.0;
}
