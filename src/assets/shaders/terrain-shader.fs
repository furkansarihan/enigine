#version 410 core

uniform vec3 lightDirection;
uniform bool wireframe;
uniform vec3 wireColor;
uniform float fogMaxDist;
uniform float fogMinDist;
uniform vec4 fogColor;
uniform float minHeight;
uniform float maxHeight;

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
in float _distance; // vertex distance to the camera
in vec3 _normal; // vertex normal

// shadowmap
in vec3 Position_worldspace;
in vec3 LightDirection_cameraspace;
in vec3 EyeDirection_cameraspace;

out vec4 color;

struct Mixer
{
    int from;
    int to;
    float mixer;
} heightMixer, slopeMixer1, slopeMixer2;

vec4 heightSteps = vec4(0, 0.004, 0.7, 1);
vec4 hTransitions = vec4(0, 0.002, 0.2, 0);
mat4 slopeStepsList = mat4(
    vec4(0, 0.1, 0.999, 1),
    vec4(0, 0.5, 0.9, 1),
    vec4(0, 0.5, 0.8, 1),
    vec4(0, 0.5, 0.8, 1)
);
mat4 sTransitions = mat4(
    vec4(0, 0.1, 0.001, 0),
    vec4(0, 0.2, 0.1, 0),
    vec4(0, 0.2, 0.1, 0),
    vec4(0, 0.2, 0.1, 0)
);

const int water = 0;
const int sand = 1;
const int stone = 2;
const int grass = 3;
const int rock = 4;
const int snow = 5;

// TODO: dirt
mat3 textureIndexes = mat3(
    vec3(rock, sand, water),
    vec3(rock, stone, grass),
    vec3(rock, snow, snow)
);

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

vec2 calculateHeightSlopeVector(float slope) {
    float maxHeightGap = maxHeight - minHeight;
    
    float h = _height / maxHeightGap;

    return vec2(h, slope);
}

Mixer calculateSampleMixer(Mixer result, float value, vec4 steps, vec4 transitions) {
    for (int i = 0; i < 3; i++) {
        if (value >= steps[i] && value < steps[i + 1]) {
            if (i != 0 && value < steps[i] + transitions[i]) {
                result.from = i - 1;
                result.to = i;
                result.mixer = (transitions[i] + value - steps[i]) / (transitions[i] * 2);
            } else if (i != 3 && value > steps[i + 1] - transitions[i + 1]) {
                result.from = i;
                result.to = i + 1;
                result.mixer = (value - (steps[i + 1] - transitions[i + 1])) / (transitions[i + 1] * 2);
            } else {
                result.from = i;
                result.to = i;
                result.mixer = 1.0;
            }

            break;
        }
    }

    return result;
}

vec3 sampleTexture(vec2 hs) {
    float h = hs.x;
    float s = hs.y;

    heightMixer = calculateSampleMixer(heightMixer, h, heightSteps, hTransitions);
    slopeMixer1 = calculateSampleMixer(slopeMixer1, s, slopeStepsList[heightMixer.from], sTransitions[heightMixer.from]);
    slopeMixer2 = calculateSampleMixer(slopeMixer2, s, slopeStepsList[heightMixer.to], sTransitions[heightMixer.to]);

    vec3 color1 = texture(textureSampler, vec3(_tuv, textureIndexes[heightMixer.from][slopeMixer1.from])).rgb;
    vec3 color2 = texture(textureSampler, vec3(_tuv, textureIndexes[heightMixer.from][slopeMixer1.to])).rgb;
    vec3 mix1 = mix(color1, color2, slopeMixer1.mixer);

    vec3 color3 = texture(textureSampler, vec3(_tuv, textureIndexes[heightMixer.to][slopeMixer2.from])).rgb;
    vec3 color4 = texture(textureSampler, vec3(_tuv, textureIndexes[heightMixer.to][slopeMixer2.to])).rgb;
    vec3 mix2 = mix(color3, color4, slopeMixer2.mixer);

    return mix(mix1, mix2, heightMixer.mixer);
}

void main()
{
    float specularMul = specularMult;

    int index = 2;
    if (gl_FragCoord.z < 0) {
    } else if (gl_FragCoord.z < FrustumDistances.x) {
        index = 0;
    } else if (gl_FragCoord.z < FrustumDistances.y) {
        index = 1;
    } else if (gl_FragCoord.z < FrustumDistances.z) {
        index = 2;
    }

    // bias = Bias[index];
    float fragToLight = dot(_normal, lightDirection);
    float bias = max(0.05 * (1.0 - fragToLight), 0.005);

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
        if (texture(ShadowMap, vec3(ShadowCoord.xy + poissonDisk[i] / 700.0, index)).x < ShadowCoord.z - bias) {
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
        float slope = dot(vec3(0, 1, 0), _normal);
        vec2 hs = calculateHeightSlopeVector(slope);
        outColor = vec4(sampleTexture(hs), 1);
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
