#version 330

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

out vec4 finalColor;

struct Light {
    int type; // 0 = Directional, 1 = Point
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float range;

    // 阴影贴图索引
    int shadowIndex;
    float shadowBias;
};

#define MAX_LIGHTS 16
#define MAX_SHADOW_CASTERS 6
#define MAX_POINT_SHADOWS 6

uniform samplerCube pointShadowMaps[MAX_POINT_SHADOWS];
uniform sampler2D shadowMaps[MAX_SHADOW_CASTERS];

uniform mat4 lightVPs[MAX_SHADOW_CASTERS];
uniform Light lights[MAX_LIGHTS];
uniform int lightCounts;
uniform vec3 emissiveColor;
uniform float emissiveIntensity;

uniform vec3 viewPos;
uniform vec4 baseColor;

uniform sampler2D u_diffuseMap;
uniform int u_diffuseMap_frameCount;
uniform float u_diffuseMap_animSpeed;
uniform float gameTime;
uniform float realTime;

float SampleShadowMap(int index, vec2 coords) {
    if(index == 0)
        return texture(shadowMaps[0], coords).r;
    if(index == 1)
        return texture(shadowMaps[1], coords).r;
    if(index == 2)
        return texture(shadowMaps[2], coords).r;
    if(index == 3)
        return texture(shadowMaps[3], coords).r;
    if(index == 4)
        return texture(shadowMaps[4], coords).r;
    if(index == 5)
        return texture(shadowMaps[5], coords).r;
    // if(index == 6)
    //     return texture(shadowMaps[6], coords).r;
    // if(index == 7)
    //     return texture(shadowMaps[7], coords).r;
    // if(index == 8)
    //     return texture(shadowMaps[8], coords).r;
    // if(index == 9)
    //     return texture(shadowMaps[9], coords).r;
    // if(index == 10)
    //     return texture(shadowMaps[10], coords).r;
    // if(index == 11)
    //     return texture(shadowMaps[11], coords).r;
    // if(index == 12)
    //     return texture(shadowMaps[12], coords).r;
    // if(index == 13)
    //     return texture(shadowMaps[13], coords).r;
    // if(index == 14)
    //     return texture(shadowMaps[14], coords).r;
    // if(index == 15)
    //     return texture(shadowMaps[15], coords).r;
    return 1.0;
}

float SamplePointShadowMap(int index, vec3 dir) {
    if(index == 0)
        return texture(pointShadowMaps[0], dir).r;
    if(index == 1)
        return texture(pointShadowMaps[1], dir).r;
    if(index == 2)
        return texture(pointShadowMaps[2], dir).r;
    if(index == 3)
        return texture(pointShadowMaps[3], dir).r;
    if(index == 4)
        return texture(pointShadowMaps[4], dir).r;
    if(index == 5)
        return texture(pointShadowMaps[5], dir).r;
    // if(index == 6)
    //     return texture(pointShadowMaps[6], dir).r;
    // if(index == 7)
    //     return texture(pointShadowMaps[7], dir).r;
    // if(index == 8)
    //     return texture(pointShadowMaps[8], dir).r;
    // if(index == 9)
    //     return texture(pointShadowMaps[9], dir).r;
    // if(index == 10)
    //     return texture(pointShadowMaps[10], dir).r;
    // if(index == 11)
    //     return texture(pointShadowMaps[11], dir).r;
    // if(index == 12)
    //     return texture(pointShadowMaps[12], dir).r;
    // if(index == 13)
    //     return texture(pointShadowMaps[13], dir).r;
    // if(index == 14)
    //     return texture(pointShadowMaps[14], dir).r;
    // if(index == 15)
    //     return texture(pointShadowMaps[15], dir).r;
    return 1.0;
}

float CalculateDirShadow(int shadowIndex, vec3 worldPos, float bias) {
    if(shadowIndex < 0 || shadowIndex >= MAX_SHADOW_CASTERS)
        return 0.0;

    vec4 fragPosLightSpace = lightVPs[shadowIndex] * vec4(worldPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    float shadow = 0.0;
    vec2 texSize = 1.0 / vec2(textureSize(shadowMaps[0], 0));

    // PCF 3x3 滤波
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = SampleShadowMap(shadowIndex, projCoords.xy + vec2(x, y) * texSize);
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}
float CalculatePointShadow(int shadowIndex, vec3 worldPos, vec3 lightPos, float farPlane, float bias) {
    if(shadowIndex < 0 || shadowIndex >= MAX_POINT_SHADOWS)
        return 0.0;

    vec3 fragToLight = worldPos - lightPos;
    float currentDepth = length(fragToLight);

    // 采样 Cubemap 存储的是线性深度 [0, 1] (由我们之前的点光源深片 Shader 写入)
    float closestDepth = SamplePointShadowMap(shadowIndex, fragToLight);

    // 还原真实距离
    closestDepth *= farPlane;

    // 简单硬阴影比较
    return (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
}
vec3 CalcLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow) {
    vec3 lightDir;
    float attenuation = 1.0;

    if(light.type == 0) { // Directional
        lightDir = normalize(-light.direction);
    } else { // Point
        lightDir = normalize(light.position - fragPos);
        float dist = length(light.position - fragPos);
        if(dist > light.range)
            return vec3(0.0);
        // 使用 range 相关的平方衰减
        attenuation = clamp(1.0 - (dist * dist) / (light.range * light.range), 0.0, 1.0);
    }

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 radiance = light.color * light.intensity * attenuation;
    return (diff + spec) * radiance * (1.0 - shadow);
}
void main() {
    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 totalLight = vec3(0.0); // 基础环境光 0.1

    for(int i = 0; i < MAX_LIGHTS; i++) {
        if(i >= lightCounts) {
            break;
        }
        if(lights[i].intensity <= 0.001)
            continue;
        float shadow = 0.0;

        if(lights[i].shadowIndex >= 0) {
            if(lights[i].type == 0) {
                shadow = CalculateDirShadow(lights[i].shadowIndex, fragPosition, lights[i].shadowBias);
            } else {
                if(lights[i].range > 0.0) {
                    shadow = CalculatePointShadow(lights[i].shadowIndex, fragPosition, lights[i].position, lights[i].range, lights[i].shadowBias);
                }
            }
        }

        totalLight += CalcLight(lights[i], norm, fragPosition, viewDir, shadow);

    }

    float currentFrame = floor(mod(gameTime * u_diffuseMap_animSpeed, float(u_diffuseMap_frameCount)));
    vec2 animatedUV = fragTexCoord;
    animatedUV.y /= u_diffuseMap_frameCount;

    animatedUV.y += (currentFrame / u_diffuseMap_frameCount);

    vec4 texColor = texture(u_diffuseMap, animatedUV);

    vec3 albedo = texture(u_diffuseMap, animatedUV).rgb * baseColor.rgb;

    float brightness = dot(texColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    vec3 emission = albedo * emissiveColor * emissiveIntensity * brightness;

    finalColor = vec4(totalLight * albedo + emission, texColor.a * baseColor.a);
}