#version 330
in vec2 fragTexCoord;
in float vNear;
in float vFar;

out vec4 finalColor;

uniform sampler2D u_fluidDepth;
uniform vec2 u_dir;

float radius = 30.0;
float sigma_s = 10.0; // 空间权重的标准差
float sigma_r = 10.5; // 范围权重的标准差

void main() {
    // vec4 centerColor = texture(rawScreen, fragTexCoord);
    float centerDepth = texture(u_fluidDepth, fragTexCoord).r;
    if(centerDepth <= 0.0 || centerDepth >= 5000.0) {
        finalColor = vec4(centerDepth, 0.0, 0.0, 1.0);
        return;
    }
    float sum = 0.0;
    float weightSum = 0.0;

    float twoSigmaS2 = 2.0 * sigma_s * sigma_s;
    float twoSigmaR2 = 2.0 * sigma_r * sigma_r;

    vec2 texSize = vec2(textureSize(u_fluidDepth, 0));
    for(float i = -radius; i <= radius; i++) {
        vec2 sampleCoord = fragTexCoord + (u_dir * i) / texSize;
        float sampleDepth = texture(u_fluidDepth, sampleCoord).r;

        //vec4 sampleColor = texture(rawScreen, sampleCoord);
        if(sampleDepth <= 0.0 || sampleDepth >= 10000.0)
            continue;
        float depthDiff = sampleDepth - centerDepth;
        float rangeWeight = exp(-(depthDiff * depthDiff) / twoSigmaR2);
        float spatialWeight = exp(-(i * i) / twoSigmaS2);
        float weight = spatialWeight * rangeWeight;

        sum += sampleDepth * weight;
        weightSum += weight;
    }
    if(weightSum > 0.0) {
        finalColor = vec4(sum / weightSum, sum / weightSum, sum / weightSum, 1.0);
    } else {
        finalColor = vec4(centerDepth, centerDepth, centerDepth, 1.0);
    }
    //finalColor = vec4(centerDepth, centerDepth, centerDepth, 1);
}