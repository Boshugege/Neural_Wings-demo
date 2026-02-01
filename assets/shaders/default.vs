#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

uniform mat4 u_mvp;
uniform mat4 transform;

out vec3 fragPosition;
out vec2 fragTexCoord;
out vec3 fragNormal;

void main() {
    fragPosition = vec3(transform * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize(vec3(transform * vec4(vertexNormal, 0.0)));
    gl_Position = u_mvp * vec4(vertexPosition, 1.0);
}