#version 410 core

uniform vec3 lightDirection;
uniform bool wireframe;
uniform vec3 wireColor;
uniform float fogMaxDist;
uniform float fogMinDist;
uniform vec4 fogColor;
uniform vec2 terrainSize;

uniform sampler2D normalMapSampler;
uniform sampler2DArray textureSampler;

// shadowmap
uniform sampler2D ShadowMap;

uniform mat4 M;
uniform mat4 V;

// uniform vec3 AmbientColor;
// uniform vec3 DiffuseColor;
// uniform vec3 SpecularColor;
uniform vec3 LightColor;
uniform float LightPower;

in float _height; // based on world coorinates
in vec2 _tuv; // texture coordinates
in vec2 _uv; // coordinates for normal-map lookup
in vec2 _zalpha; // coordinates for elevation-map lookup
in float _distance; // vertex distance to the camera

// shadowmap
in vec4 ShadowCoord;
in vec3 Position_worldspace;
in vec3 LightDirection_cameraspace;
in vec3 EyeDirection_cameraspace;
// in vec3 Normal_cameraspace;

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
    // do a texture lookup to get the normal in current level
    vec4 normalfc = texture(normalMapSampler, _uv);
    // normal_fc.xy contains normal at current (fine) level
    // normal_fc.zw contains normal at coarser level
    // blend normals using alpha computed in vertex shader  
    vec3 normal = vec3((1 - _zalpha.y) * normalfc.xy + _zalpha.y * (normalfc.zw), 1.0);

    // unpack coordinates from [0, 1] to [-1, +1] range, and renormalize.
    normal = normalize(normalfc.xyz * 2 - 1);

    float s = clamp(dot(normal, lightDirection), 0.8, 1);

    // Normal of the computed fragment, in camera space
	vec3 n = normalize((V * M * vec4(normal, 0)).xyz);
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

    float bias = 0.05 * tan(acos(cosTheta));
    bias = clamp(bias, 0, 0.05);
    // bias = 0.1;

    float visibility = 1.0;

    // 1. sampler2D basic
    // float nearestOccluderDist = texture(ShadowMap, ShadowCoord.xy).x;
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
    for (int i = 0; i < 4; i++) {
        // 1. simple
        // float nearestOccluderDist = texture(ShadowMap, ShadowCoord.xy + poissonDisk[i] / 700.0).x;
        // if (nearestOccluderDist < ShadowCoord.z - bias) {
        //     visibility -= 0.2;
        // }
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
		int index = int(16.0 * random(floor(Position_worldspace.xyz * 1000.0), i)) % 16;
		
		// being fully in the shadow will eat up 4*0.2 = 0.8
		// 0.2 potentially remain, which is quite dark.
		visibility -= 0.2 * (1.0 - texture(ShadowMap, vec2(ShadowCoord.xy + poissonDisk[index] / 700.0)).x);
    }

    // TODO: check outside of shadowmap

    vec4 outColor;
    if (wireframe) {
        float distanceRate = (_distance / 10000);
        float normalRate = 1 - distanceRate;
        outColor = vec4(wireColor, 1 * normalRate);
    } else {
        // outColor = vec4(texture(textureSampler, _tuv).rgb * s, 1); // sampler2D
        if (_height == 0) {
            int index = 0; // water
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb * s, 1); // sampler2DArray
        } else if (_height >= 0 && _height < 0.5) {
            vec3 mergedColor = transitionRegion(0, 1, 0.5, 0.5);
            outColor = vec4(mergedColor * s, 1);
        } else if (_height < 1) {
            int index = 1; // sand
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb * s, 1);
        } else if (_height >= 1 && _height < 2) {
            vec3 mergedColor = transitionRegion(1, 2, 2, 1);
            outColor = vec4(mergedColor * s, 1);
        } else if (_height < 8) {
            int index = 2; // stone
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb * s, 1);
        } else if (_height >= 8 && _height < 9) {
            vec3 mergedColor = transitionRegion(2, 3, 9, 1);
            outColor = vec4(mergedColor * s, 1);
        } else if (_height < 50) {
            int index = 3; // grass
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb * s, 1);
        } else if (_height >= 50 && _height < 54) {
            vec3 mergedColor = transitionRegion(3, 4, 54, 4);
            outColor = vec4(mergedColor * s, 1);
        } else if (_height < 160) {
            int index = 4; // rock
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb * s, 1);
        } else if (_height >= 160 && _height < 164) {
            vec3 mergedColor = transitionRegion(4, 5, 164, 4);
            outColor = vec4(mergedColor * s, 1);
        } else {
            int index = 5; // snow
            outColor = vec4(texture(textureSampler, vec3(_tuv, index)).rgb * s, 1);
        }
    }

    // vec3 DiffuseColor = texture(textureSampler, vec3(_tuv / 1024, 0)).rgb * s;
    vec3 DiffuseColor = outColor.xyz;

    // TODO:
    // vec3 col =
    //     // Ambient : simulates indirect lighting
    //     // AmbientColor +
    //     // Diffuse : "color" of the object
    //     visibility * DiffuseColor * LightColor * LightPower * cosTheta;
    //     // Specular : reflective highlight, like a mirror
    //     // visibility * SpecularColor * LightColor * LightPower * pow(cosAlpha, 5);
    // color = vec4(col, 1);
    outColor = vec4(visibility * DiffuseColor, 1);

    // fog
    float fogFactor = (fogMaxDist - _distance) / (fogMaxDist - fogMinDist);
    fogFactor = clamp(fogFactor, 0, 1);
    color = mix(fogColor, outColor, fogFactor);
}
