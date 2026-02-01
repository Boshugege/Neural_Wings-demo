#version 330
in vec3 fragPosition;
in vec3 fragNormal;

uniform vec3 viewPos;
uniform vec4 baseColor;

uniform float near = 0.1;
uniform float far = 10000.0;

uniform float realTime;
uniform float gameTime;

out vec4 finalColor;

float LinearizeDepth(float depth) {
  float z = depth * 2.0 - 1.0;
  return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
  // vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
  // float diff = max(dot(fragNormal, lightDir), 0.0);
  // vec3 diffuse = baseColor.rgb * diff;
  // float pulse = (sin(realTime * 5.0) * 0.5 + 0.5) * 0.1;
  // finalColor = vec4(diffuse + vec3(pulse), baseColor.a);
  float depth = gl_FragColor.z;
  float linearDepth = LinearizeDepth(depth);
  float vis = linearDepth / 50.0;
  finalColor = vec4(vec3(vis), 1.0);
}