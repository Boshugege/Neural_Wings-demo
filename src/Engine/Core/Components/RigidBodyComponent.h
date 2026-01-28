#pragma once
#include "IComponent.h"
#include "raylib.h"
#include "raymath.h"
#include "Engine/Math/Math.h"
#include <functional>
#include <utility>

enum class ColliderType
{
    NONE,
    SPHERE,
    BOX,
};
class GameObject;

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

    float elasticity = 1.0f; // 弹性系数
    bool Collidable = false;
    ColliderType colliderType = ColliderType::NONE;
    std::pair<Vector3f, Vector3f> localBoundingBox = std::make_pair(Vector3f(0.0f, 0.0f, 0.0f),
                                                                    Vector3f(0.0f, 0.0f, 0.0f));
    float boudingRadius = 0.0f;
    std::function<void(GameObject *)> collisionCallback;

    void setHitboxBox(const Vector3f &min, const Vector3f &max)
    {
        localBoundingBox.first = min;
        localBoundingBox.second = max;
        colliderType = ColliderType::BOX;
        Collidable = true;
    }
    void setHitboxBox(const Vector3f &size)
    {
        setHitboxBox(-size / 2.0f, size / 2.0f);
    }
    void setHitboxSphere(const float &radius)
    {
        boudingRadius = radius;
        colliderType = ColliderType::SPHERE;
        Collidable = true;
    }

    float mass = 1.0f;
    float drag = 0.5f; // 线性空气阻力

    Matrix3f inertiaTensor = Matrix3f::identity();            // 惯性张量
    Matrix3f inverseInertiaTensor = Matrix3f::identity();     // 逆惯性张量
    float angularDrag = 0.01f;                                // 角空气阻力
    Vector3f angularVelocity = Vector3f(0.0f, 0.0f, 0.0f);    // 角速度
    Vector3f angularMomentum = Vector3f(0.0f, 0.0f, 0.0f);    // 角动量
    Vector3f accumulatedTorques = Vector3f(0.0f, 0.0f, 0.0f); // 当前扭矩

    Vector3f velocity = Vector3f(0.0f, 0.0f, 0.0f);  // 当前速度
    Vector3f acceleration = {0.0f, 0.0f, 0.0f};      // 当前加速度
    Vector3f accumulatedForces = {0.0f, 0.0f, 0.0f}; // 当前合力

    void SetAnglularVelocity(Vector3f angularVelocity, Quat4f rotation = Quat4f::IDENTITY)
    {
        this->angularVelocity = angularVelocity;
        Matrix3f rotationMatrix = rotation.toMatrix();
        Matrix3f worldInertia = rotationMatrix * (this->inertiaTensor) * rotationMatrix.transposed();

        this->angularMomentum = worldInertia * (this->angularVelocity);
    }

    void SetBox(Vector3f size)
    {
        SetBoxInertia(size);
        setHitboxBox(size);
    }
    void SetSphere(float radius)
    {

        SetSphereInertia(radius);
        setHitboxSphere(radius);
    }
    void SetSphereInertia(float radius)
    {
        if (mass <= 0.0f)
            return;

        float i = (2.0f / 5.0f) * mass * radius * radius;
        inverseInertiaTensor = Matrix3f::identity();
        inverseInertiaTensor.setDiag(1.0f / i, 1.0f / i, 1.0f / i);
        inertiaTensor = Matrix3f::identity();
        inertiaTensor.setDiag(i, i, i);
    }
    // I = 1/12 * m * (h*h + d*d)
    void SetBoxInertia(Vector3f size, Quat4f rotation = Quat4f::IDENTITY)
    {
        if (mass <= 0.0f)
            return;

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

        Matrix3f rotationMatrix = rotation.toMatrix();
        Matrix3f worldInertia = rotationMatrix * (this->inertiaTensor) * rotationMatrix.transposed();
        this->angularMomentum = worldInertia * (this->angularVelocity);
    }
    void SetDiagInertia(float xx, float yy, float zz, Quat4f rotation = Quat4f::IDENTITY)
    {
        inverseInertiaTensor = Matrix3f::identity();
        inverseInertiaTensor.setDiag(1.0f / xx, 1.0f / yy, 1.0f / zz);
        inertiaTensor = Matrix3f::identity();
        inertiaTensor.setDiag(xx, yy, zz);

        Matrix3f rotationMatrix = rotation.toMatrix();
        Matrix3f worldInertia = rotationMatrix * (this->inertiaTensor) * rotationMatrix.transposed();
        this->angularMomentum = worldInertia * (this->angularVelocity);
    }
    // 施加冲量
    void AddImpulse(Vector3f impulse, Vector3f r = Vector3f::ZERO)
    {
        float invMass = mass > 0.01f ? 1.0f / mass : 0.0f;
        this->velocity += impulse * invMass;
        if (invMass > 0.01f)
            this->angularMomentum += r ^ impulse;
    }
    // 施加力
    void AddForce(Vector3f force)
    {
        accumulatedForces += force;
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