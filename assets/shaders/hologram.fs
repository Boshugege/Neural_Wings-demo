#version 330
in vec3 fragPosition;
in vec3 fragNormal;
uniform float realTime;
uniform vec4 baseColor;
out vec4 finalColor;

void main() {
    float scanline = sin(fragPosition.y * 10.0 - realTime * 10.0);
    scanline = step(0.9, scanline);

    vec3 normal = normalize(fragNormal);
    float edge = 1.0 - abs(normal.z);

    float flicker = sin(realTime * 50.0) > 0.9 ? 0.5 : 1.0;

    vec3 holoColor = vec3(0.2, 1.0, 0.4);
    float finalAlpha = (edge * 0.5 + scanline * 0.5) * flicker;

    finalColor = vec4(holoColor, finalAlpha);
}