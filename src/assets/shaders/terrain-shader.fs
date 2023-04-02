#version 410 core

uniform vec3 lightDirection;
uniform bool wireframe;
uniform vec3 wireColor;
uniform float fogMaxDist;
uniform float fogMinDist;
uniform vec4 fogColor;
uniform vec2 terrainSize;

uniform sampler2DArray textureSampler;
uniform sampler2DArray ShadowMap;

uniform mat4 M;
uniform mat4 V;

uniform float ambientMult;
uniform float diffuseMult;
uniform float specularMult;

uniform vec3 CamPos;
uniform vec3 CamView;
uniform vec4 FrustumDistances;
uniform bool ShowCascade;
uniform vec3 Bias;

layout (std140) uniform matrices {
    mat4 DepthBiasVP[3];
};

in float _height; // based on world coorinates
in vec2 _tuv; // texture coordinates
in vec2 _zalpha; // coordinates for elevation-map lookup
in float _distance; // vertex distance to the camera
in vec3 _normal; // vertex normal

// shadowmap
in vec3 Position_worldspace;
in vec3 LightDirection_cameraspace;
in vec3 EyeDirection_cameraspace;

out vec4 color;

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

vec3 transitionRegion(int from, int to, float regionEnd, float regionSize) {
    vec3 color1 = texture(textureSampler, vec3(_tuv, from)).rgb;
    vec3 color2 = texture(textureSampler, vec3(_tuv, to)).rgb;

    float dist = regionEnd - _height;
    float mixFactor = dist / regionSize;

    return mix(color2, color1, mixFactor);
}

void main()
{
    float specularMul = specularMult;

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

    vec4 ShadowCoord = DepthBiasVP[index] * vec4(Position_worldspace, 1);

    // Normal of the computed fragment, in camera space
    vec3 n = normalize((V * vec4(_normal, 0)).xyz);
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

    // float bias = 0.05 * tan(acos(cosTheta));
    // bias = clamp(bias, 0, 0.05);
    // bias = 0.1;

    float visibility = 1.0;

    // 1. sampler2D basic
    float nearestOccluderDist = texture(ShadowMap, vec3(ShadowCoord.xy, index)).x;
    // vec4 outColor = vec4(vec3(nearestOccluderDist), 1.0);
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
        if (texture(ShadowMap, vec3(ShadowCoord.xy + poissonDisk[i] / 700.0, index)).x < ShadowCoord.z - Bias[index]) {
            visibility -= 0.08;
        }

        // 2. simple2
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
        // visibility -= 0.2 * (1.0 - texture(ShadowMap, vec3(ShadowCoord.xy + poissonDisk[index] / 700.0, 2)).x);
    }

    // Z comparision doesn't work expected when two distance is too close to each other
    // So, we're making fragment fully visible when it's far enough
    if (nearestOccluderDist > 0.9) {
        visibility = 1.0;
    }

    vec4 outColor = vec4(0.8, 0.8, 0.8, 1.0);
    if (wireframe) {
        float distanceRate = (_distance / 10000);
        float normalRate = 1 - distanceRate;
        outColor = vec4(wireColor, 1 * normalRate);
    } else {
        // outColor = vec4(texture(textureSampler, _tuv).rgb, 1); // sampler2D
        if (_height == 0) {
            specularMul *= 10;
            int index = 0; // water
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb, 1); // sampler2DArray
        } else if (_height >= 0 && _height < 0.5) {
            float start = 0;
            float gap = _height - start;
            specularMul *= (gap * 5 + 10);
            vec3 mergedColor = transitionRegion(0, 1, 0.5, 0.5);
            outColor = vec4(mergedColor, 1);
        } else if (_height < 1) {
            int index = 1; // sand
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb, 1);
        } else if (_height >= 1 && _height < 2) {
            vec3 mergedColor = transitionRegion(1, 2, 2, 1);
            outColor = vec4(mergedColor, 1);
        } else if (_height < 8) {
            int index = 2; // stone
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb, 1);
        } else if (_height >= 8 && _height < 9) {
            vec3 mergedColor = transitionRegion(2, 3, 9, 1);
            outColor = vec4(mergedColor, 1);
        } else if (_height < 50) {
            int index = 3; // grass
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb, 1);
        } else if (_height >= 50 && _height < 54) {
            vec3 mergedColor = transitionRegion(3, 4, 54, 4);
            outColor = vec4(mergedColor, 1);
        } else if (_height < 160) {
            int index = 4; // rock
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb, 1);
        } else if (_height >= 160 && _height < 164) {
            vec3 mergedColor = transitionRegion(4, 5, 164, 4);
            outColor = vec4(mergedColor, 1);
        } else {
            int index = 5; // snow
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb, 1);
        }
    }

    vec3 DiffuseColor = outColor.xyz;
    vec3 SpecularColor = vec3(1, 1, 1);
    vec3 LightColor = vec3(0.2, 0.2, 0.2);
    float LightPower = 10;

    // Ambient : simulates indirect lighting
    vec3 ambient = DiffuseColor * ambientMult;
    // Diffuse : "color" of the object
    vec3 diffuse = visibility * ambient * LightColor * LightPower * cosTheta * diffuseMult;
    // Specular : reflective highlight, like a mirror
    vec3 specular = visibility * SpecularColor * LightColor * LightPower * pow(cosAlpha, 5) * specularMul;

    vec3 col = ambient + diffuse + specular;
    outColor = vec4(col, 1);

    if (ShowCascade) {
        outColor[index] = 0.9;
    }

    // fog
    float fogFactor = (fogMaxDist - _distance) / (fogMaxDist - fogMinDist);
    fogFactor = clamp(fogFactor, 0, 1);
    color = mix(fogColor, outColor, fogFactor);
}
