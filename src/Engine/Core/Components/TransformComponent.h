#pragma once
#include "IComponent.h" 
#include "raylib.h"
// TODO: 数学库替换
struct TransformComponent : public IComponent {
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    Quaternion rotation = { 0.0f, 0.0f, 0.0f, 1.0f }; 
    Vector3 scale = { 1.0f, 1.0f, 1.0f };

    TransformComponent() = default;
    TransformComponent(const Vector3& pos) : position(pos) {}
    TransformComponent(const Vector3& pos, const Quaternion& rot, const Vector3& scl)
        : position(pos), rotation(rot), scale(scl) {}
};