#pragma once
#include "IComponent.h"
#include "raylib.h"
#include "Engine/Math/Math.h"

struct TransformComponent : public IComponent
{
    Vector3f position = Vector3f(0.0f, 0.0f, 0.0f);
    Quat4f rotation = Quat4f(1.0f, 0.0f, 0.0f, 0.0f);
    Vector3f scale = Vector3f(1.0f, 1.0f, 1.0f);

    TransformComponent() = default;
    TransformComponent(const Vector3f &pos) : position(pos) {}
    TransformComponent(const Vector3f &pos, const Quat4f &rot, const Vector3f &scl)
        : position(pos), rotation(rot), scale(scl) {}
};