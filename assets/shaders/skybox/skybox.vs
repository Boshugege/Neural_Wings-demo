#version 330

in vec3 vertexPosition;

uniform mat4 matView;
uniform mat4 matProj;

out vec3 fragPosition;

void main() {
    fragPosition = vertexPosition;
    mat4 rotView = mat4(mat3(matView));

    vec4 clipPos = matProj * rotView * vec4(vertexPosition, 1.0);
    gl_Position = clipPos.xyww;

}