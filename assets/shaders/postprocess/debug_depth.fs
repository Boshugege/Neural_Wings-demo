#version 330
in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D rawScreen_depth;
uniform sampler2D rawScreen;
uniform float near;
uniform float far;

float LinerDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near * far) / (far + near - z * (far - near)); // Linearize
}

void main() {
    float depth = texture(rawScreen_depth, fragTexCoord).r;
    vec4 color = texture(rawScreen, fragTexCoord);
    float linearDepth = LinerDepth(depth);
    float display = linearDepth / 50;

    finalColor = vec4(display, display, display, 1.0);
}