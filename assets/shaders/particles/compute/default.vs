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
in uint pID;

// 标准输出
out vec4 outPosition; // (x,y,z,padding)
out vec4 outVelocity; // (x,y,z,padding)
out vec4 outAcceleration; // (x,y,z,padding)

out vec4 outColor;
out vec4 outSizeRotation; // (s1,s2,rotation,padding)
out vec4 outLifeRand; // (totalLife,remainingLife,randomID,ID )

// 引擎内置参数
uniform sampler2D dataTex;
uniform int maxParticles;
uniform float deltaTime;
uniform float gameTime;
uniform float realTime;

vec3 GetPos(int id) {
    return texelFetch(dataTex, ivec2(0, id), 0).xyz;
}
vec3 GetVel(int id) {
    return texelFetch(dataTex, ivec2(1, id), 0).xyz;
}
vec4 GetAcc(int id) {
    return texelFetch(dataTex, ivec2(2, id), 0);
}
vec4 GetColor(int id) {
    return texelFetch(dataTex, ivec2(3, id), 0);
}
vec4 GetSize(int id) {
    return texelFetch(dataTex, ivec2(4, id), 0);
}
vec4 GetLife(int id) {
    return texelFetch(dataTex, ivec2(5, id), 0);
}

void main() {
    float dt = deltaTime;
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
    outLifeRand = vec4(pLife.x, remaingLife, pRandomID, pID);
}