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

// 自定参数
uniform float u_bloomThreshold;

void main() {
    vec4 color = texture(screenTexture, fragTexCoord);
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > u_bloomThreshold) {
        finalColor = color;
    } else {
        finalColor = vec4(1.0);
    }
}