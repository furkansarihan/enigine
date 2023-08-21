#version 410 core

out vec4 FragColor;

in vec3 localPos;
  
uniform samplerCube environmentMap;
uniform vec3 sunDirection;
uniform vec3 sunColor;
  
void main()
{
    vec3 envColor = texture(environmentMap, localPos).rgb;

    // Calculate the sun glow based on the angle between the sun direction and view direction
    vec3 viewDir = normalize(localPos);
    float dotProduct = dot(viewDir, sunDirection);
    
    // Define an angular threshold to determine the sun's appearance
    float sun = 0.99995;
    float sunReach = 0.5;

    vec3 color = sunColor;
    float gradient;
    if (dotProduct < sunReach) {
        gradient = 0.0;
    } else if (dotProduct < sun) {
        gradient = smoothstep(sunReach, sun, (dotProduct - sunReach) / (sun - sunReach));
        gradient /= 128;
    } else {
        gradient = 1.0;
        color *= 100;
    }

    // Combine the environment color and sun color using the gradient
    vec3 finalColor = mix(envColor, color, gradient);

    FragColor = vec4(finalColor, 1.0);
}
