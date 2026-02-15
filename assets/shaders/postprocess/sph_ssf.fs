#version 330
in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D u_normal;
uniform sampler2D u_simpleFluid;
uniform sampler2D u_thickness;
uniform sampler2D u_fluidDepth;
uniform sampler2D u_sceneTex;
uniform samplerCube skyboxMap;

uniform vec3 cameraPosition;
uniform vec3 cameraDir;
uniform vec3 cameraUp;
uniform vec3 cameraRight;
uniform float cameraFov;
uniform vec2 screenResolution;

uniform vec3 lightDir = normalize(vec3(-0.5, -1.0, -0.5));

vec3 getWorldPos(vec2 uv) {
    float d = texture(u_fluidDepth, uv).r;
    if(d <= 0.0 || d >= 1000.0)
        return vec3(0.0);
    float radFov = radians(cameraFov);
    float tanHalfFov = tan(radFov * 0.5);

    vec2 ndc = uv * 2.0 - 1;
    float aspect = float(screenResolution.x) / float(screenResolution.y);
    vec3 rayDir = cameraDir +
        (cameraRight * ndc.x * aspect * tanHalfFov) +
        (cameraUp * ndc.y * tanHalfFov);
    vec3 worldRay = normalize(rayDir);
    return cameraPosition + worldRay * d;

}

void main() {
    vec3 worldPos = getWorldPos(fragTexCoord);
    // vec4 centerColor = texture(rawScreen, fragTexCoord);
    vec4 screenColor = texture(u_sceneTex, fragTexCoord);
    float thickness = texture(u_thickness, fragTexCoord).r;
    float depth = texture(u_fluidDepth, fragTexCoord).r;
    vec4 fluidColor = texture(u_simpleFluid, fragTexCoord);
    if(depth <= 0.01 || depth >= 1000.0) {
        finalColor = screenColor;
        return;
    }

    vec3 viewDir = normalize(worldPos - cameraPosition);
    vec3 normal = texture(u_normal, fragTexCoord).rgb * 2.0 - 1.0;

//折射  
    float refractionStrength = 0.05;
    vec2 refractionUV = fragTexCoord + normal.xy * refractionStrength * min(1.0, thickness);
    vec3 refractionColor = texture(u_sceneTex, refractionUV).rgb;

// 吸收
    vec3 liquidColor = vec3(0.1, 0.5, 0.8);
    float absorption = exp(-thickness * 0.2);
    vec3 transmittedColor = mix(liquidColor, refractionColor, absorption);

//高光
    vec3 r = viewDir - normal * 2.0 * dot(normal, viewDir);

    float spec = pow(max(dot(-lightDir, r), 0.0), 128.0);

    vec3 specularColor = vec3(1.0) * spec;

// 反射

    vec3 envColor = texture(skyboxMap, r).rgb;
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0 / 2.2));
    float fresnel = 0.02 + 0.98 * pow(1.0 - max(dot(normal, -viewDir), 0.0), 5.0);

    float brightness = dot(fluidColor.rgb, vec3(0.2126, 0.7152, 0.0722));

    vec3 finalRGB = mix(transmittedColor, envColor, fresnel) + specularColor + fluidColor.rgb * 0.0;

    float edgeAlpha = smoothstep(0.00, 0.1, thickness);

    finalColor = vec4(finalRGB, edgeAlpha);
}