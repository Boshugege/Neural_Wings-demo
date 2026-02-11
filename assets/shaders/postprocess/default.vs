#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;

uniform float cameraNear;
uniform float cameraFar;

out vec2 fragTexCoord;
out vec4 fragColor;
out float vNear;
out float vFar;
void main() {
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
    vNear = cameraNear;
    vFar = cameraFar;
}
