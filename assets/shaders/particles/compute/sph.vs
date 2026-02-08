#version 330

// 标准输入
in vec3 pPosition;
in vec3 pVelocity;
in vec3 pAcceleration;

in vec4 pColor;
in vec2 pSize;
in float pRotation;
in vec2 pLife; // (totalLife,remainingLife )
in uint pRandomID;

// 标准输出
out vec4 outPosition; // (x,y,z,padding)
out vec4 outVelocity; // (x,y,z,padding)
out vec4 outAcceleration; // (x,y,z,padding)

out vec4 outColor;
out vec4 outSizeRotation; // (s1,s2,rotation,padding)
out vec4 outLifeRand; // (totalLife,remainingLife,randomID,padding )

// 自定义输入
uniform float u_deltaTime;

void main() {
    float dt = u_deltaTime;
    vec3 newVelocity = pVelocity + (pAcceleration) * dt;
    newVelocity *= 0.99;
    float vel = length(newVelocity);
    vec3 newPosition = pPosition + newVelocity * dt;
    float remaingLife = pLife.y - dt;
    vec2 newSize = pSize * 0.99;

    outPosition = vec4(newPosition, 0);
    outVelocity = vec4(newVelocity, 0);
    outAcceleration = vec4(pAcceleration, 0);

    outColor = pColor;
    outSizeRotation = vec4(newSize, pRotation + 10 * vel * dt, 0);
    outLifeRand = vec4(pLife.x, remaingLife, pRandomID, 0);
}