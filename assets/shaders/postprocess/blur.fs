#version 330
in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D u_screen;
uniform vec2 u_direction;
uniform float u_radius;

const float weights[5] = float[](0.14446, 0.23336, 0.13553, 0.05135, 0.00994);
const float offsets[5] = float[](0.00000, 1.41176, 3.29411, 5.17647, 7.05882);

void main() {
    vec2 tex_size = textureSize(u_screen, 0);
    vec2 tex_offset = 1.0 / tex_size;

    vec3 result = texture(u_screen, fragTexCoord).rgb * weights[0];

    for(int i = 1; i < 5; i++) {
        vec2 offset = u_direction * tex_offset * offsets[i] * u_radius;
        result += texture(u_screen, fragTexCoord + offset).rgb * weights[i];
        result += texture(u_screen, fragTexCoord - offset).rgb * weights[i];
    }
    finalColor = vec4(result, 1.0);
}