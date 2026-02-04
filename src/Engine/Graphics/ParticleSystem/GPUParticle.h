#pragma once
#include "Engine/Math/Math.h"
#pragma pack(push, 1)
struct GPUParticle
{

    Vector3f position;      // loc 0:vec3
    float spaceFlag = 0.0f; // loc 1 0.0为随体系，1.0为世界系

    Vector3f velocity;     // loc 2
    float padding1 = 0.0f; // padding to align to 16 bytes

    Vector3f acceleration; // loc 3
    float padding2 = 0.0f;

    Vector4f color; // loc 4

    Vector2f size;  // loc 5
    float rotation; // loc 6
    float padding3 = 0.0f;

    Vector2f life;         // loc 7 (totalLife,remainingLife )
    unsigned int randomID; // loc 8
    float padding4 = 0.0f;
};
#pragma pack(pop)