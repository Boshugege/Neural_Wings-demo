#version 330

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D u_baseTex;
uniform sampler2D u_gtaoTex;

void main() {
    vec4 sceneColor = texture(u_baseTex, fragTexCoord);
    float ao = texture(u_gtaoTex, fragTexCoord).r;

    vec3 toneColor = sceneColor.rgb * ao;

    fragColor = vec4(toneColor, sceneColor.a);
}