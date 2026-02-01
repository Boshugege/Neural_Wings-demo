#version 330
in vec3 fragPosition;
in vec3 fragNormal;
in float v_linearDepth;

uniform vec3 viewPos;
uniform vec4 baseColor;

uniform float realTime;
uniform float gameTime;

out vec4 finalColor;

void main() {
    float range = 20.0;
    float d = v_linearDepth / range;
    finalColor = vec4(vec3(d), 1);
}
