#version 330 core

// Final composite: original scene + bloom

in vec2 fragUV;
out vec4 FragColor;

uniform sampler2D uScene;
uniform sampler2D uBloom;
uniform float uBloomStrength;
uniform float uExposure;

void main() {
    vec3 scene = texture(uScene, fragUV).rgb;
    vec3 bloom = texture(uBloom, fragUV).rgb;

    // Additive blend
    vec3 color = scene + bloom * uBloomStrength;

    // ACES filmic tone mapping
    vec3 x = color * uExposure;
    color = (x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14);
    color = clamp(color, 0.0, 1.0);

    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
