#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D u_screen;
uniform sampler2D u_screen_depth;
uniform vec3 cameraPosition;
uniform vec3 cameraDir;
uniform mat4 invVP;
uniform float realTime;

// 雾参数
uniform vec3 u_dustColor;
uniform float u_density;

uniform float cameraFov;
uniform vec2 screenResolution;
uniform float cameraFar;
uniform float cameraNear;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform float gameTime;

float hash(vec3 p) {
    p = fract(p * 0.1031);
    p += dot(p, p.yzx + 33.33);
    return fract((p.x + p.y) * p.z);
}

float noise3D(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(mix(hash(i + vec3(0, 0, 0)), hash(i + vec3(1, 0, 0)), f.x), mix(hash(i + vec3(0, 1, 0)), hash(i + vec3(1, 1, 0)), f.x), f.y), mix(mix(hash(i + vec3(0, 0, 1)), hash(i + vec3(1, 0, 1)), f.x), mix(hash(i + vec3(0, 1, 1)), hash(i + vec3(1, 1, 1)), f.x), f.y), f.z);
}

vec3 getWorldPos(vec2 uv) {
    float d = texture(u_screen_depth, uv).r;
    float z = d * 2.0 - 1.0;
    float linearZ = (2.0 * cameraNear * cameraFar) / (cameraFar + cameraNear - z * (cameraFar - cameraNear));
    float aspect = screenResolution.x / screenResolution.y;
    float tanHalfFov = tan(radians(cameraFov) * 0.5);
    vec2 ndc = uv * 2.0 - 1.0;

    vec3 viewPos;
    viewPos.x = ndc.x * tanHalfFov * aspect * linearZ;
    viewPos.y = ndc.y * tanHalfFov * linearZ;
    viewPos.z = -linearZ;

    vec4 clipPos = vec4(ndc, z, 1.0);
    vec4 worldPos = invVP * clipPos;
    return worldPos.xyz / worldPos.w;
}

void main() {
    vec4 sceneColor = texture(u_screen, fragTexCoord);

    vec3 worldPos = getWorldPos(fragTexCoord);

    float dist = length(worldPos - cameraPosition);

    // 体积密度计算
    float fogFactor = 1.0 - exp(-dist * u_density);

    float dustGrains = 0.5 * 0.2 * (1.0 - fogFactor);

    vec3 finalRGB = mix(sceneColor.rgb, u_dustColor, fogFactor);
    finalRGB += u_dustColor * dustGrains;

    finalColor = vec4(finalRGB, sceneColor.a);
}