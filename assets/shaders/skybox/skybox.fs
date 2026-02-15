#version 330

in vec3 fragPosition;
out vec4 finalColor;

uniform samplerCube skyboxMap;
uniform vec4 tintColor;

void main() {
    vec3 color = texture(skyboxMap, fragPosition).rgb;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    finalColor = vec4(color, 1.0) * tintColor;
}