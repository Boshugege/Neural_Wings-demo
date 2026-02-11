#version 330
in vec2 fragTexCoord;
out vec4 finalColor;

uniform vec3 cameraPosition;
uniform vec3 cameraDir;
uniform vec3 cameraUp;
uniform vec3 cameraRight;
uniform float cameraFov;
uniform vec2 screenResolution;

uniform sampler2D u_fluidDepth;

vec3 getWorldPos(vec2 uv) {
    float d = texture(u_fluidDepth, uv).r;
    if(d <= 0.0 || d >= 1000.0)
        return vec3(0.0);
    float radFov = radians(cameraFov);
    float tanHalfFov = tan(radFov * 0.5);

    vec2 ndc = uv * 2.0 - vec2(1, -1);
    float aspect = float(screenResolution.x) / float(screenResolution.y);
    vec3 rayDir = cameraDir +
        (cameraRight * ndc.x * aspect * tanHalfFov) +
        (cameraUp * ndc.y * tanHalfFov);
    vec3 worldRay = normalize(rayDir);
    return cameraPosition + worldRay * d;

}

void main() {
    // vec4 centerColor = texture(rawScreen, fragTexCoord);
    float centerDepth = texture(u_fluidDepth, fragTexCoord).r;
    if(centerDepth <= 0.01 || centerDepth >= 5000.0) {
        discard;
    }
    vec3 pos = getWorldPos(fragTexCoord);
    vec2 texelSize = 1.0 / textureSize(u_fluidDepth, 0);

    float dR = texture(u_fluidDepth, fragTexCoord + vec2(texelSize.x, 0.0)).r;
    float dL = texture(u_fluidDepth, fragTexCoord - vec2(texelSize.x, 0.0)).r;
    float dU = texture(u_fluidDepth, fragTexCoord + vec2(0.0, texelSize.y)).r;
    float dD = texture(u_fluidDepth, fragTexCoord - vec2(0.0, texelSize.y)).r;

    vec3 posR = getWorldPos(fragTexCoord + vec2(texelSize.x, 0.0));
    vec3 posL = getWorldPos(fragTexCoord - vec2(texelSize.x, 0.0));
    vec3 posU = getWorldPos(fragTexCoord + vec2(0.0, texelSize.y));
    vec3 posD = getWorldPos(fragTexCoord - vec2(0.0, texelSize.y));

    float threshold = 0.5;

    vec3 ddx, ddy;

    float dzR = abs(dR - centerDepth);
    float dzL = abs(dL - centerDepth);
    if(dzR < dzL && dzR < threshold) {
        ddx = posR - pos;
    } else if(dzL < threshold) {
        ddx = pos - posL;
    } else {
        ddx = vec3(texelSize.x, 0.0, 0.0);
    }

    float dzU = abs(dU - centerDepth);
    float dzD = abs(dD - centerDepth);
    if(dzU < dzD && dzU < threshold) {
        ddy = posU - pos;
    } else if(dzD < threshold) {
        ddy = pos - posD;
    } else {
        ddy = vec3(0.0, texelSize.y, 0.0);
    }

    vec3 normal = normalize(cross(ddx, ddy));
    finalColor = vec4(normal * 0.5 + 0.5, 1.0);
}