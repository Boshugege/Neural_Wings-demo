#version 330

// 标准输入
in vec3 pPosition;
in float spaceFlag;
in vec3 pVelocity;
in vec3 pAcceleration;

in vec4 pColor;
in vec2 pSize;
in float pRotation;
in vec2 pLife; // (totalLife,remainingLife )
in uint pRandomID;

// 标准输出
out vec3 outPosition;
out float outSpaceFlag;
out vec3 outVelocity;
out vec3 outAcceleration;

out vec4 outColor;
out vec2 outSize;
out float outRotation;
out vec2 outLife; // (totalLife,remainingLife )
out uint outRandomID;

// 自定义输入
uniform float u_deltaTime;

void main() {
    float dt = u_deltaTime;
    // ....
    vec3 newVelocity = pVelocity + (pAcceleration + vec3(0.0, -9.8, 0.0)) * dt;
    vec3 newPosition = pPosition + newVelocity * dt;
    float remaingLife = pLife.y - dt;

    outPosition = newPosition;
    outSpaceFlag = spaceFlag;
    outVelocity = newVelocity;
    outAcceleration = pAcceleration;
    outColor = pColor;
    outSize = pSize;
    outRotation = pRotation + 0.1 * dt; // 测试旋转
    outLife = vec2(pLife.x, remaingLife);
    outRandomID = pRandomID;
}