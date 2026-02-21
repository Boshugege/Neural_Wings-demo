#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

// 引擎内置参数
uniform float gameTime;
uniform float realTime;
uniform float deltaRealTime;
uniform float deltaGameTime;
uniform vec2 screenResolution;

// 自定义参数
uniform sampler2D u_screen;

void main() {
    vec3 result = texture(u_screen, fragTexCoord).rgb;
    finalColor = vec4(result, 1.0);
}