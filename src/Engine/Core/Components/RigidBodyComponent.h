#pragma once
#include "IComponent.h"
#include "raylib.h"
#include "raymath.h"

struct RigidbodyComponent : public IComponent
{
    RigidbodyComponent(const float &mass = 1.0f,
                       const float &drag = 0.0f,
                       const Vector3 &velocity = {0.0f, 0.0f, 0.0f},
                       const Vector3 &acceleration = {0.0f, 0.0f, 0.0f})
        : mass(mass),
          drag(drag),
          velocity(velocity),
          acceleration(acceleration) {}
    RigidbodyComponent(const RigidbodyComponent &other) = default;

    ~RigidbodyComponent() = default;
    float mass = 1.0f;
    float drag = 0.5f; // 线性空气阻力
    // TODO: 添加角动量相关参数

    Vector3 velocity = {0.0f, 0.0f, 0.0f};          // 当前速度
    Vector3 acceleration = {0.0f, 0.0f, 0.0f};      // 当前加速度
    Vector3 accumulatedForces = {0.0f, 0.0f, 0.0f}; // 本帧受到的合力

    void SetParameters(float mass, float drag)
    {
        this->mass = mass;
        this->drag = drag;
    }
    // 施加力
    void AddForce(Vector3 force)
    {
        accumulatedForces = Vector3Add(accumulatedForces, force);
    }

    // 清空受力 (每帧结束时调用)
    void ClearForces()
    {
        accumulatedForces = {0.0f, 0.0f, 0.0f};
    }
};