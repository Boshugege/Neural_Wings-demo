#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

// 引擎内置参数
uniform float gameTime;
uniform float realTime;
uniform float deltaRealTime;
uniform float deltaGameTime;
uniform sampler2D screenTexture;
uniform vec2 screenResolution;

// 自定义参数
uniform vec2 u_direction;
uniform float u_radius;

const float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
const float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

void main() {
    vec2 tex_offset = 1.0 / screenResolution;
    vec3 result = texture(screenTexture, fragTexCoord).rgb * weight[0];

    for(int i = 1; i < 3; i++) {
        vec2 sampleOffset = u_direction * tex_offset * offset[i] * u_radius;
        result += texture(screenTexture, fragTexCoord + sampleOffset).rgb * weight[i];
        result += texture(screenTexture, fragTexCoord - sampleOffset).rgb * weight[i];
    }
    finalColor = vec4(result, 1.0);
}