#version 330

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D u_baseTex;
uniform sampler2D u_sparks;
uniform sampler2D u_RTTest1;
uniform sampler2D u_RTTest2;

uniform float u_bloomIntensity;

void main() {
    vec4 sceneColor = texture(u_baseTex, fragTexCoord);
    vec4 sparkColor = texture(u_sparks, fragTexCoord);
    vec4 RTtestColor1 = texture(u_RTTest1, fragTexCoord);
    vec4 RTtestColor2 = texture(u_RTTest2, fragTexCoord);

    vec3 toneColor = RTtestColor1.rgb + RTtestColor2.rgb * 0.0 + sceneColor.rgb + sparkColor.rgb * u_bloomIntensity;

    fragColor = vec4(toneColor, sceneColor.a);
}