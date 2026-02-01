#version 330
in vec3 fragPosition;
in vec3 fragNormal;

uniform vec3 viewPos;
uniform vec4 baseColor;
uniform float realTime;

out vec4 finalColor;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);

    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(normal, lightDir), 0.0);

    float rim = 1.0 - max(dot(normal, viewDir), 0.0);
    rim = pow(rim, 3.0); // 调整幂次改变边缘的宽度

    float pulse = (sin(realTime * 2.0) * 0.5 + 0.5) * 0.3;
    vec3 rimColor = vec3(0.0, 0.6, 1.0);

    vec3 finalRGB = baseColor.rgb * (diff + 0.2) + (rimColor * (rim + pulse));
    finalColor = vec4(finalRGB, baseColor.a);
}