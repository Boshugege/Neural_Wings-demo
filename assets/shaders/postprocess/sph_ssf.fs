#version 330
in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D u_normal;
uniform sampler2D u_simpleFluid;
uniform sampler2D u_thickness;
uniform sampler2D u_fluidDepth;
uniform sampler2D u_sceneTex;

uniform vec3 lightDir = normalize(vec3(-0.5, -1.0, -0.5));
uniform vec3 cameraDir;

void main() {
    // vec4 centerColor = texture(rawScreen, fragTexCoord);
    vec4 screenColor = texture(u_sceneTex, fragTexCoord);
    float thickness = texture(u_thickness, fragTexCoord).r;
    float depth = texture(u_fluidDepth, fragTexCoord).r;
    vec4 fluidColor = texture(u_simpleFluid, fragTexCoord);
    if(depth <= 0.01 || depth >= 1000.0) {
        finalColor = screenColor;
        return;
    }
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
    vec3 r = cameraDir - normal * 2.0 * dot(normal, cameraDir);

    float spec = pow(max(dot(-lightDir, r), 0.0), 128.0);

    vec3 specularColor = vec3(1.0) * spec;

// 反射
    vec3 viewDir = cameraDir;

    vec4 envColor = vec4(1, 1, 1, 1);
    float fresnel = 0.02 + 0.98 * pow(1.0 - max(dot(normal, -viewDir), 0.0), 5.0);
    if(r.y <= 0)
        envColor = vec4(0.0, 0.72, 1.0, 1.0);
    else
        envColor = vec4(0.25, 0.15, 0.1, 1.0);
    vec3 envReflection = envColor.rgb * fresnel;

    // vec3 finalRGB = mix(transmittedColor, envReflection, fresnel) + specularColor;
    // vec3 skyColor = vec3(1.0);
    // vec3 groundColor = vec3(0.13, 0.11, 0.11);
    float brightness = dot(fluidColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.5)
        specularColor += vec3(0.);

    vec3 finalRGB = mix(transmittedColor, envReflection, fresnel) + specularColor + fluidColor.rgb * 0.0;

    float edgeAlpha = smoothstep(0.01, 0.1, thickness);
    // vec4 debugColor = screenColor * 1 + thickColor * 0 + depthColor * 0 + fluidColor * 0 + normalColor * 1;

    finalColor = vec4(finalRGB, edgeAlpha);
}