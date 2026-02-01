#version 330
in vec3 fragPosition;
in vec3 fragNormal;
in float v_linearDepth;

uniform vec3 viewPos;
uniform vec4 baseColor;

uniform float near = 0.1;
uniform float far = 1000.0;

uniform float realTime;
uniform float gameTime;

out vec4 finalColor;

void main() {
    vec3 viewDir = normalize(viewPos - fragPosition);
    finalColor = vec4(viewDir * 0.5 + 0.5, 1);
}
