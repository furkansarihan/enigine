// This file originally created by SpineyPete.
// https://gist.github.com/SpineyPete/ebf9619f009318536c6da48209894fed

// Tonemapping
// -----------
// Some of these are attributed to certain games.
// But this is mostly secondhand info, so take it with a grain of salt.

vec3 pow3(vec3 x, float exponent) {
    return vec3(pow(x.r, exponent), pow(x.g, exponent), pow(x.b, exponent));
}

vec3 Tonemap_Reinhard(vec3 x) {

    // Simplest Reinhard tonemapper.
    return x / (1.0 + x);
}

vec3 Tonemap_Simple(vec3 x) {

    // Reinhard with toe.
    x = pow3(x, 1.25);  // Toe
    return x/(x + 1.0); // Shoulder
}

vec3 Tonemap_Simple2(vec3 x) {

    // I used this one to add HDR to an LDR authored game.
    // The 0.5 point meets up with linear.
    x = x * exp(x);
    return x / (x + 0.6);
}

vec3 Tonemap_Reinhard_lab(vec3 lab) {

    // "Desaturated" Reinhard for formats with
    // lightness stored in the r channel.
    // Total hack but doesn't look all bad.
    lab.x = lab.x / (1.0 + lab.x);
    return lab;
}

vec3 Tonemap_UE3(vec3 x) {

    // Used in Unreal Engine 3 up to 4.14. (I think, might be wrong).
    // They've since moved to ACES for output on a larger variety of devices.
    // Very simple and intented for use with color-lut afterwards.
    return x / (x + 0.187) * 1.035;
}

vec3 Tonemap_Photographic(vec3 x) {

    // Simple photographic tonemapper.
    // Suggested by Emil Persson on Beyond3D forum.
    return 1.0 - exp2(-x);
}

vec3 Tonemap_Filmic_UC2(vec3 linearColor, float linearWhite,
    float A, float B, float C, float D, float E, float F) {

    // Uncharted II configurable tonemapper.

    // A = shoulder strength
    // B = linear strength
    // C = linear angle
    // D = toe strength
    // E = toe numerator
    // F = toe denominator
    // Note: E / F = toe angle
    // linearWhite = linear white point value

    vec3 x = linearColor;
    vec3 color = ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
    
    x = vec3(linearWhite);
    vec3 white = ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
    
    return color / white;
}

vec3 Tonemap_Filmic_UC2Default(vec3 linearColor) {

    // Uncharted II fixed tonemapping formula.
    // Gives a warm and gritty image, saturated shadows and bleached highlights.

    return Tonemap_Filmic_UC2(linearColor, 11.2, 0.22, 0.3, 0.1, 0.2, 0.01, 0.30);
}

vec3 Tonemap_Filmic_UC2DefaultToGamma(vec3 linearColor) {

    // Uncharted II fixed tonemapping formula.
    // The linear to sRGB conversion is baked in.

    vec3 x = max(vec3(0), linearColor - vec3(0.004));
    return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

vec3 Tonemap_Aces(vec3 color) {

    // ACES filmic tonemapper with highlight desaturation ("crosstalk").
    // Based on the curve fit by Krzysztof Narkowicz.
    // https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/

    const float slope = 12.0f; // higher values = slower rise.

    // Store grayscale as an extra channel.
    vec4 x = vec4(
        // RGB
        color.r, color.g, color.b,
        // Luminosity
        (color.r * 0.299) + (color.g * 0.587) + (color.b * 0.114)
    );
    
    // ACES Tonemapper
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;

    vec4 tonemap = clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
    float t = x.a;
    
    t = t * t / (slope + t);

    // Return after desaturation step.
    return mix(tonemap.rgb, tonemap.aaa, t);
}

vec3 Tonemap_AcesApprox(vec3 x) {
    // Approximate version of the above, no crosstalk.
    const float toe = 1.25f;
    const float shoulder = 1.88f;
    x = pow3(x, toe);
    return x/(x + shoulder);
}
