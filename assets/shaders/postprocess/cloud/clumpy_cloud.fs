#version 330
in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D u_screen;
uniform sampler2D u_screen_depth;
uniform mat4 invVP;
uniform vec3 cameraPosition;
uniform float cameraFar;
uniform float cameraNear;
uniform float realTime;

uniform float u_cloudFrequency = 0.05;
uniform float u_cloudSize = 0.45;
uniform float u_cloudThickness = 0.2;
uniform vec3 u_windDir = vec3(1.0, 0.0, 0.0);
uniform float u_windSpeed = 2.0;
uniform float u_stretch = 1.5;
uniform float u_distVar = 0.5;
uniform float u_density;

uniform float MAX_STEPS;
uniform float MAX_DIST;
uniform float STEP_SIZE;

float hash(vec3 p) {
    p = fract(p * 0.56673);
    p += dot(p, p.yzx + 33.33);
    return fract((p.x + p.y) * p.z);
}

float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(mix(hash(i + vec3(0, 0, 0)), hash(i + vec3(1, 0, 0)), f.x), mix(hash(i + vec3(0, 1, 0)), hash(i + vec3(1, 1, 0)), f.x), f.y), mix(mix(hash(i + vec3(0, 0, 1)), hash(i + vec3(1, 0, 1)), f.x), mix(hash(i + vec3(0, 1, 1)), hash(i + vec3(1, 1, 1)), f.x), f.y), f.z);
}
float fbm(vec3 p) {
    float v = 0.0;
    float amp = 0.5;
    for(int i = 0; i < 4; i++) {
        v += amp * noise(p);
        p *= 2.02;
        amp *= 0.5;
    }
    return v;
}

float LinearizeDepth(vec2 uv) {
    float d = texture(u_screen_depth, uv).r;
    float z = d * 2.0 - 1.0;
    return (2.0 * cameraNear * cameraFar) / (cameraFar + cameraNear - z * (cameraFar - cameraNear));
}
void main() {
    vec4 sceneColor = texture(u_screen, fragTexCoord);
    vec4 farPos = invVP * vec4(fragTexCoord * 2.0 - 1.0, 1.0, 1.0);
    vec3 rayDir = normalize(farPos.xyz / farPos.w - cameraPosition);

    float sceneDistance = LinearizeDepth(fragTexCoord);
    float marchDist = min(sceneDistance, MAX_DIST);

    float transmittance = 1.0;
    vec3 scatteredLight = vec3(0.0);

    float offset = hash(vec3(fragTexCoord * 1000.0, realTime)) * STEP_SIZE;
    for(int i = 0; i < MAX_STEPS; i++) {
        float d = float(i) * STEP_SIZE + offset;
        if(d > marchDist)
            break;
        vec3 p = cameraPosition + rayDir * d;
        vec3 windNormalized = normalize(u_windDir);

        vec3 pWarped = p - windNormalized * dot(p, windNormalized) * (1.0 - 1.0 / u_stretch);
        pWarped -= u_windDir * realTime * u_windSpeed;

        float coverage = noise(pWarped * u_cloudFrequency * 0.2);
        float dynamicSize = u_cloudSize + (0.5 - coverage) * u_distVar;

        float baseDensity = fbm(pWarped * u_cloudFrequency);
        float density = smoothstep(dynamicSize, dynamicSize + u_cloudThickness, baseDensity);

        if(density > 0.0) {
            float detail = fbm(pWarped * 0.3 + realTime * 0.1);
            density *= detail;
        }

        density *= u_density;
        if(density > 0.001) {
            vec3 localColor = mix(vec3(1.0, 1.0, 1.1), vec3(0.8, 0.9, 1.0), density * 2.0);
            float transparency = exp(-density * STEP_SIZE);
            scatteredLight += localColor * (transmittance * (1.0 - transparency));
            transmittance *= transparency;
        }
        if(transmittance < 0.01)
            break;
    }
    vec3 finalRGB = sceneColor.rgb * transmittance + scatteredLight;
    finalColor = vec4(finalRGB, sceneColor.a);
}
