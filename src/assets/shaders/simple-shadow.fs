#version 410 core

out vec4 color;

uniform sampler2DArray ShadowMap;

uniform vec3 AmbientColor;
uniform vec3 DiffuseColor;
uniform vec3 SpecularColor;
uniform vec3 LightColor;
uniform float LightPower;

uniform float biasMult;
uniform mat4 M;
uniform vec3 CamPos;
uniform vec3 CamView;
uniform vec4 FrustumDistances;
uniform bool ShowCascade;

layout (std140) uniform matrices {
    mat4 DepthBiasVP[3];
};

in vec3 VertexPosition_modelspace;
in vec3 Position_worldspace;
in vec3 LightDirection_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 Normal_cameraspace;

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i){
    vec4 seed4 = vec4(seed,i);
    float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

void main()
{
    vec3 ambientColor = AmbientColor;

    vec3 v = Position_worldspace - CamPos;
    float fragToCamDist = dot(v, CamView);

    int index = 4;
    if (fragToCamDist < 0) {
    } else if (fragToCamDist < FrustumDistances.x) {
        index = 0;
    } else if (fragToCamDist < FrustumDistances.y) {
        index = 1;
    } else if (fragToCamDist < FrustumDistances.z) {
        index = 2;
    }

    if (ShowCascade) {
        ambientColor[index] *= 0.8;
    }

    mat4 DepthBiasMVP = DepthBiasVP[index] * M;
    vec4 ShadowCoord = DepthBiasMVP * vec4(VertexPosition_modelspace, 1);

    // color = vec4(DiffuseColor.xyz * visibility, 1);

    // vec3 LightColor = vec3(1,1,1);
    // float LightPower = 1.0;

    // Material properties
    // vec3 DiffuseColor = texture( myTextureSampler, UV ).rgb;
    // vec3 AmbientColor = vec3(0.1, 0.1, 0.1) * DiffuseColor;
    // vec3 SpecularColor = vec3(0.3, 0.3, 0.3);

    // Normal of the computed fragment, in camera space
    vec3 n = normalize(Normal_cameraspace);
    // Direction of the light (from the fragment to the light)
    vec3 l = normalize(LightDirection_cameraspace);
    // Cosine of the angle between the normal and the light direction, 
    // clamped above 0
    //  - light is at the vertical of the triangle -> 1
    //  - light is perpendiular to the triangle -> 0
    //  - light is behind the triangle -> 0
    float cosTheta = clamp(dot(n, l), 0, 1);

    // Eye vector (towards the camera)
    vec3 E = normalize(EyeDirection_cameraspace);
    // Direction in which the triangle reflects the light
    vec3 R = reflect(-l, n);
    // Cosine of the angle between the Eye vector and the Reflect vector,
    // clamped to 0
    //  - Looking into the reflection -> 1
    //  - Looking elsewhere -> < 1
    float cosAlpha = clamp(dot(E, R), 0, 1);

    float bias = biasMult * tan(acos(cosTheta));
    bias = clamp(bias, 0, 0.01);

    float visibility = 1.0;

    // 1. sampler2D basic
    float nearestOccluderDist = texture(ShadowMap, vec3(ShadowCoord.xy, index)).x;
    // float fragDist = ShadowCoord.z;
    // 
    // if (nearestOccluderDist < fragDist - bias){
    //     visibility = 0.5;
    // }

    // 2. sampler2DShadow
    // Compute the depth comparison value
    // float shadowValue = texture(ShadowMap, ShadowCoord.xyz, bias);
    // // Use the depth comparison value to determine if the fragment is in shadow
    // if (ShadowCoord.w > shadowValue)
    // {
    //     visibility = 0.5;
    // }

    // 3. Poission sampling
    for (int i = 0; i < 8; i++) {
        // 1. simple
        if (texture(ShadowMap, vec3(ShadowCoord.xy + poissonDisk[i] / 700.0, index)).x < ShadowCoord.z - bias) {
            visibility -= 0.08;
        }

        // 2. advanced
        // use either :
        //  - Always the same samples.
        //    Gives a fixed pattern in the shadow, but no noise
        // int index = i;
        //  - A random sample, based on the pixel's screen location. 
        //    No banding, but the shadow moves with the camera, which looks weird.
        // int index = int(16.0 * random(gl_FragCoord.xyy, i)) % 16;
        //  - A random sample, based on the pixel's position in world space.
        //    The position is rounded to the millimeter to avoid too much aliasing
        // int index = int(16.0 * random(floor(Position_worldspace.xyz * 1000.0), i)) % 16;

        // being fully in the shadow will eat up 4*0.2 = 0.8
        // 0.2 potentially remain, which is quite dark.
        // visibility -= 0.2 *
        //     (1.0 - texture(ShadowMap, 
        //             vec2(ShadowCoord.xy + poissonDisk[index] / 700.0)).x);
    }

    // Z comparision doesn't work expected when two distance is too close to each other
    // So, we're making fragment fully visible when it's far enough
    if (nearestOccluderDist > 0.9) {
        visibility = 1.0;
    }

    vec3 col =
        // Ambient : simulates indirect lighting
        ambientColor +
        // Diffuse : "color" of the object
        visibility * DiffuseColor * LightColor * LightPower * cosTheta +
        // Specular : reflective highlight, like a mirror
        visibility * SpecularColor * LightColor * LightPower * pow(cosAlpha, 5);
    // TODO: adapt to hdr
    color = vec4(col * 2.2, 1);

    // color = vec4(vec3(nearestOccluderDist), 1);
    // color = vec4(vec3(fragDist), 1);
}
