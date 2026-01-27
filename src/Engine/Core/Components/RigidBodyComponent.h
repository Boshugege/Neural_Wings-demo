#pragma once
#include "IComponent.h"
#include "raylib.h"
#include "raymath.h"
#include "Engine/Math/Math.h"
struct RigidbodyComponent : public IComponent
{
    RigidbodyComponent(const float &mass = 1.0f,
                       const float &drag = 0.0f,
                       const Vector3f &velocity = Vector3f(0.0f, 0.0f, 0.0f),
                       const Vector3f &acceleration = Vector3f(0.0f, 0.0f, 0.0f))
        : mass(mass),
          drag(drag),
          velocity(velocity),
          acceleration(acceleration) {}
    RigidbodyComponent(const RigidbodyComponent &other) = default;

    ~RigidbodyComponent() = default;
    float mass = 1.0f;
    float drag = 0.5f; // 线性空气阻力
    // TODO: 添加角动量相关参数

    Matrix3f inertiaTensor = Matrix3f::identity(); // 惯性张量
    Matrix3f inverseInertiaTensor =  Matrix3f::identity();// 逆惯性张量
    float angularDrag = 0.01f; // 角空气阻力
    Vector3f angularVelocity = Vector3f(0.0f, 0.0f, 0.0f); // 角速度
    Vector3f angularMomentum = Vector3f(0.0f, 0.0f, 0.0f); // 角动量
    Vector3f accumulatedTorques = Vector3f(0.0f, 0.0f, 0.0f); // 当前扭矩

    Vector3f velocity = Vector3f(0.0f, 0.0f, 0.0f);          // 当前速度
    Vector3f acceleration = {0.0f, 0.0f, 0.0f};      // 当前加速度
    Vector3f accumulatedForces = {0.0f, 0.0f, 0.0f}; // 当前合力

    void SetAnglularVelocity(Vector3f angularVelocity,Quat4f rotation=Quat4f::IDENTITY) {
        this->angularVelocity = angularVelocity;
        Matrix3f rotationMatrix = rotation.toMatrix();
        Matrix3f worldInertia = rotationMatrix * (this->inertiaTensor) * rotationMatrix.transposed();

        this->angularMomentum= worldInertia * (this->angularVelocity);
    }
    // --- 长方体的惯性张量 ---
    // I = 1/12 * m * (h*h + d*d)
    void SetBoxInertia(Vector3f size) {
        if (mass <= 0.0f) return;

        float x2 = size.x() * size.x();
        float y2 = size.y() * size.y();
        float z2 = size.z() * size.z();

        float ixx = (1.0f / 12.0f) * mass * (y2 + z2);
        float iyy = (1.0f / 12.0f) * mass * (x2 + z2);
        float izz = (1.0f / 12.0f) * mass * (x2 + y2);

        inverseInertiaTensor = Matrix3f::identity();
        inverseInertiaTensor.setDiag(1.0f / ixx, 1.0f / iyy, 1.0f / izz);
        inertiaTensor = Matrix3f::identity();
        inertiaTensor.setDiag(ixx, iyy, izz);
    }
    void SetDiagInertia(float xx, float yy, float zz) {
        inverseInertiaTensor = Matrix3f::identity();
        inverseInertiaTensor.setDiag(1.0f / xx, 1.0f / yy, 1.0f / zz);
        inertiaTensor = Matrix3f::identity();
        inertiaTensor.setDiag(xx, yy, zz);
    }
    // 施加力
    void AddForce(Vector3f force)
    {
        accumulatedForces +=  force;
    }
    void AddTorque(Vector3f torque)
    {
        accumulatedTorques += torque;
    }
    // 清空受力 (每帧结束时调用)
    void ClearForces()
    {
        accumulatedForces = Vector3f(0.0f, 0.0f, 0.0f);
        accumulatedTorques = Vector3f(0.0f, 0.0f, 0.0f);
    }
};