#version 330
uniform vec4 u_color;
uniform float realTime;

out vec4 finalColor;

void main() {
    float brightness = 0.8 + 0.5 * sin(realTime * 20.0);
    vec3 b = clamp(u_color.rgb * brightness / 255.0, 0.0, 1.0);
    finalColor = vec4(b, u_color.a / 255.0);
}