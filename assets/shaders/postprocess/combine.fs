#version 330

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D u_baseTex;
uniform sampler2D u_brightTex;

uniform float u_bloomIntensity;

void main() {
    vec4 sceneColor = texture(u_baseTex, fragTexCoord);
    vec4 bloomColor = texture(u_brightTex, fragTexCoord);

    vec3 toneColor = sceneColor.rgb + bloomColor.rgb * u_bloomIntensity;

    fragColor = vec4(toneColor, sceneColor.a);
}