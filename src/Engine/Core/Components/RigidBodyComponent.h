#pragma once
#include "IComponent.h"
#include "raylib.h"
#include "raymath.h"
#include "Engine/Math/Math.h"
#include <iostream>
#include <limits>

class GameObject;
class TransformComponent;
enum class ColliderType
{
    NONE,
    SPHERE,
    BOX,
};

struct AABB
{
    Vector3f min;
    Vector3f max;
    AABB() : min(Vector3f(0.0f, 0.0f, 0.0f)), max(Vector3f(0.0f, 0.0f, 0.0f)) {}
    AABB(const Vector3f &min, const Vector3f &max) : min(min), max(max) {}
    static bool IsCollide(const AABB &a, const AABB &b)
    {
        bool collision = true;

        if ((a.max.x() >= b.min.x()) && (a.min.x() <= b.max.x()))
        {
            if ((a.max.y() < b.min.y()) || (a.min.y() > b.max.y()))
                collision = false;
            if ((a.max.z() < b.min.z()) || (a.min.z() > b.max.z()))
                collision = false;
        }
        else
            collision = false;

        return collision;
    }
};

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
    AABB localAABB = AABB(Vector3f(0.0f, 0.0f, 0.0f),
                          Vector3f(0.0f, 0.0f, 0.0f));
    float boudingRadius = 0.0f;
    std::function<void(GameObject *)> collisionCallback;

    void setHitboxBox(const Vector3f &min, const Vector3f &max)
    {
        localAABB.min = min;
        localAABB.max = max;
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
    void SetHitbox(Vector3f size)
    {
        if (colliderType == ColliderType::BOX)
            SetBox(size);
        else if (colliderType == ColliderType::SPHERE)
            SetSphere(size);
        else
            std::cerr << "No hitbox set" << std::endl;
    }
    void SetBox(Vector3f size)
    {
        SetBoxInertia(size);
        setHitboxBox(size);
    }
    void SetSphere(Vector3f radius)
    {

        SetSphereInertia(radius.x());
        setHitboxSphere(radius.x());
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
        float invMass = mass > std::numeric_limits<float>::min() ? 1.0f / mass : 0.0f;
        this->velocity += impulse * invMass;
        if (invMass > std::numeric_limits<float>::min())
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

struct HitBox
{
    ColliderType colliderType = ColliderType::BOX;
    Vector3f center;
    // BOX
    Vector3f axes[3];
    Vector3f halfExtents;
    // Sphere
    float boudingRadius;
    HitBox(const TransformComponent &tf, const RigidbodyComponent &rb)
    {
        switch (rb.colliderType)
        {
        case ColliderType::SPHERE:
            setHitboxSphere(tf.GetLocalPosition(), tf.GetLocalRotation(), rb.boudingRadius);
            break;
        case ColliderType::BOX:
            setHitboxBox(tf.GetLocalPosition(), tf.GetLocalRotation(), rb.localAABB);
            break;
        default:
            break;
        }
    }
    void setHitboxSphere(Vector3f center, Quat4f rotation, float radius)
    {
        this->center = center;
        this->boudingRadius = radius;
        this->colliderType = ColliderType::SPHERE;
        Matrix3f rotMat = rotation.toMatrix();
        for (int i = 0; i < 3; i++)
            axes[i] = rotMat.getCol(i);
    }
    void setHitboxBox(Vector3f center, Quat4f rotation, AABB localBoundingBox)
    {
        this->center = center;
        Vector3f size = localBoundingBox.max - localBoundingBox.min; // hitbox size
        this->halfExtents = size * 0.5f;
        this->colliderType = ColliderType::BOX;

        Matrix3f rotMat = rotation.toMatrix();
        for (int i = 0; i < 3; i++)
            axes[i] = rotMat.getCol(i);
    }
    static float GetProjectionRadius(const HitBox &a, const Vector3f &axis)
    {
        float res = 0.0f;
        for (int i = 0; i < 3; i++)
            res += a.halfExtents[i] * fabsf(a.axes[i] * axis);
        return res;
    }
    static bool TestAxis(const HitBox &a, const HitBox &b, Vector3f axis,
                         Vector3f &outNormal, float &outMinPenetration, int curAxisIndex, int &outBestIndex)
    {
        if (axis.LengthSquared() < 0.0001f)
            return false;
        axis.Normalize();

        float distProj = fabsf((b.center - a.center) * axis);
        float sumRadius = GetProjectionRadius(a, axis) + GetProjectionRadius(b, axis);

        float pen = sumRadius - distProj;
        if (pen <= 0.05f)
            return true; // 分离
        if (pen < outMinPenetration)
        {
            outMinPenetration = pen;
            outNormal = axis;
            outBestIndex = curAxisIndex;
        }
        return false;
    }

    static bool GetCollisionInfo(const HitBox &a, const HitBox &b,
                                 Vector3f &normal, float &penetration, Vector3f &hitPoint)
    {
        if (a.colliderType == ColliderType::BOX && b.colliderType == ColliderType::BOX)
            return GetCollisionInfoBOXBOX(a, b, normal, penetration, hitPoint);
        if (a.colliderType == ColliderType::SPHERE && b.colliderType == ColliderType::SPHERE)
            return GetCollisionInfoSphereSphere(a, b, normal, penetration, hitPoint);
        if (a.colliderType == ColliderType::SPHERE && b.colliderType == ColliderType::BOX)
            return GetCollisionInfoSphereBox(a, b, normal, penetration, hitPoint);
        if (a.colliderType == ColliderType::BOX && b.colliderType == ColliderType::SPHERE)
            return GetCollisionInfoBoxSphere(a, b, normal, penetration, hitPoint);
        std::cerr << "[RigidBodyComponent] Unhandled collision type combination!" << std::endl;
        return false;
    }

    static bool GetCollisionInfoBoxSphere(const HitBox &box, const HitBox &sphere,
                                          Vector3f &normal, float &penetration, Vector3f &hitPoint)
    {
        // 球心在 Box 局部坐标系下的位置
        Vector3f centerDelta = sphere.center - box.center;
        float localX = centerDelta * box.axes[0];
        float localY = centerDelta * box.axes[1];
        float localZ = centerDelta * box.axes[2];

        // 寻找 Box 表面或内部距离球心最近的点
        float closestLocalX = std::max(-box.halfExtents[0], std::min(localX, box.halfExtents[0]));
        float closestLocalY = std::max(-box.halfExtents[1], std::min(localY, box.halfExtents[1]));
        float closestLocalZ = std::max(-box.halfExtents[2], std::min(localZ, box.halfExtents[2]));

        // 最近点换回世界坐标
        Vector3f closestPoint = box.center +
                                box.axes[0] * closestLocalX +
                                box.axes[1] * closestLocalY +
                                box.axes[2] * closestLocalZ;

        // 最近点与球心的距离
        Vector3f toSphere = sphere.center - closestPoint;
        float distSq = toSphere.LengthSquared();

        if (distSq > sphere.boudingRadius * sphere.boudingRadius)
            return false;

        float dist = sqrtf(distSq);

        if (dist < 1e-5f)
        {
            // 球心在 Box 内部

            // 计算到各个面的距离
            float dX = box.halfExtents[0] - fabsf(localX);
            float dY = box.halfExtents[1] - fabsf(localY);
            float dZ = box.halfExtents[2] - fabsf(localZ);

            if (dX < dY && dX < dZ)
            {
                // X轴最近
                normal = box.axes[0] * (localX < 0 ? -1.0f : 1.0f);
                penetration = dX + sphere.boudingRadius;
            }
            else if (dY < dZ)
            {
                // Y轴最近
                normal = box.axes[1] * (localY < 0 ? -1.0f : 1.0f);
                penetration = dY + sphere.boudingRadius;
            }
            else
            {
                // Z轴最近
                normal = box.axes[2] * (localZ < 0 ? -1.0f : 1.0f);
                penetration = dZ + sphere.boudingRadius;
            }

            // normal 指向 Box 外部
            hitPoint = sphere.center - normal * sphere.boudingRadius;
        }
        else
        {
            // 球心在 Box 外部
            normal = toSphere / dist; // 指向 Sphere
            penetration = sphere.boudingRadius - dist;
            hitPoint = closestPoint;
        }

        return true;
    }

    static bool GetCollisionInfoSphereBox(const HitBox &sphere, const HitBox &box,
                                          Vector3f &normal, float &penetration, Vector3f &hitPoint)
    {
        // 球心在 Box 局部坐标系下的位置
        Vector3f centerDelta = sphere.center - box.center;
        float localX = centerDelta * box.axes[0];
        float localY = centerDelta * box.axes[1];
        float localZ = centerDelta * box.axes[2];

        // 寻找 Box 表面或内部距离球心最近的点
        float closestLocalX = std::max(-box.halfExtents[0], std::min(localX, box.halfExtents[0]));
        float closestLocalY = std::max(-box.halfExtents[1], std::min(localY, box.halfExtents[1]));
        float closestLocalZ = std::max(-box.halfExtents[2], std::min(localZ, box.halfExtents[2]));

        // 最近点换回世界坐标
        Vector3f closestPoint = box.center +
                                box.axes[0] * closestLocalX +
                                box.axes[1] * closestLocalY +
                                box.axes[2] * closestLocalZ;

        // 最近点与球心的距离
        Vector3f toSphere = sphere.center - closestPoint;
        float distSq = toSphere.LengthSquared();

        if (distSq > sphere.boudingRadius * sphere.boudingRadius)
            return false;

        float dist = sqrtf(distSq);

        if (dist < 1e-5f)
        {
            // 球心在 Box 内部

            // 计算到各个面的距离
            float dX = box.halfExtents[0] - fabsf(localX);
            float dY = box.halfExtents[1] - fabsf(localY);
            float dZ = box.halfExtents[2] - fabsf(localZ);

            if (dX < dY && dX < dZ)
            {
                // X轴最近
                normal = box.axes[0] * (localX < 0 ? -1.0f : 1.0f);
                penetration = dX + sphere.boudingRadius;
            }
            else if (dY < dZ)
            {
                // Y轴最近
                normal = box.axes[1] * (localY < 0 ? -1.0f : 1.0f);
                penetration = dY + sphere.boudingRadius;
            }
            else
            {
                // Z轴最近
                normal = box.axes[2] * (localZ < 0 ? -1.0f : 1.0f);
                penetration = dZ + sphere.boudingRadius;
            }

            // normal 指向 Box 外部
            hitPoint = sphere.center - normal * sphere.boudingRadius;
        }
        else
        {
            // 球心在 Box 外部
            normal = -toSphere / dist; // 指向 BOX
            penetration = sphere.boudingRadius - dist;
            hitPoint = closestPoint;
        }

        return true;
    }
    static bool GetCollisionInfoSphereSphere(const HitBox &a, const HitBox &b,
                                             Vector3f &normal, float &penetration, Vector3f &hitPoint)
    {
        Vector3f delta = b.center - a.center;
        float distSq = delta.LengthSquared();
        float sumRadius = a.boudingRadius + b.boudingRadius;

        if (distSq >= sumRadius * sumRadius)
            return false;

        float dist = sqrtf(distSq);
        if (dist < 1e-5f)
        {
            penetration = sumRadius;
            normal = Vector3f(1, 0, 0); // 任意轴
            hitPoint = a.center;
        }
        else
        {
            penetration = sumRadius - dist;
            normal = delta / dist;
            hitPoint = a.center + normal * a.boudingRadius;
        }
        return true;
    }
    static bool GetCollisionInfoBOXBOX(const HitBox &a, const HitBox &b,
                                       Vector3f &normal, float &penetration, Vector3f &hitPoint)
    {
        float minPenetration = std::numeric_limits<float>::max();
        Vector3f bestAxis;
        int bestAxisIndex = -1;
        // 0-2:face A
        // 3-5:face B
        // 6-14:edge
        for (int i = 0; i < 3; i++)
            if (TestAxis(a, b, a.axes[i], bestAxis, minPenetration, i, bestAxisIndex))
                return false;
        for (int i = 0; i < 3; i++)
            if (TestAxis(a, b, b.axes[i], bestAxis, minPenetration, i + 3, bestAxisIndex))
                return false;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                if (TestAxis(a, b, a.axes[i] ^ b.axes[j], bestAxis, minPenetration, 6 + i * 3 + j, bestAxisIndex))
                    return false;
        penetration = minPenetration;
        normal = bestAxis;
        if (normal * (b.center - a.center) < 0)
            normal = -normal;

        if (bestAxisIndex < 6)
        {
            const HitBox *refBox;
            const HitBox *otherBox;
            Vector3f serachDir;
            if (bestAxisIndex < 3)
            {
                refBox = &a;
                otherBox = &b;
                // B上接触点方向-normal
                serachDir = -normal;
            }
            else
            {
                refBox = &b;
                otherBox = &a;
                serachDir = normal;
            }
            hitPoint = HitBox::GetSupportPoint(*otherBox, serachDir);
        }
        else
        {
            int i = (bestAxisIndex - 6) / 3;
            int j = (bestAxisIndex - 6) % 3;
            auto eDirA = a.axes[i];
            auto eDirB = b.axes[j];

            auto ptA = HitBox::GetSupportPoint(a, normal);
            auto ptB = HitBox::GetSupportPoint(b, -normal);

            auto p1 = ptA + eDirA * a.halfExtents[i];
            auto q1 = ptA - eDirA * a.halfExtents[i];

            auto p2 = ptB + eDirB * b.halfExtents[j];
            auto q2 = ptB - eDirB * b.halfExtents[j];
            hitPoint = HitBox::GetContactPointEdgeEdge(p1, q1, p2, q2);
        }
        return true;
    }

    static Vector3f GetSupportPoint(const HitBox &a, const Vector3f &dir)
    {
        Vector3f res = a.center;
        for (int i = 0; i < 3; i++)
        {
            float sign = dir * a.axes[i] < 0 ? -1.0f : 1.0f;
            res += a.axes[i] * sign * a.halfExtents[i];
        }
        return res;
    }
    static Vector3f GetContactPointEdgeEdge(const Vector3f &p1, const Vector3f &q1, const Vector3f &p2, const Vector3f &q2)
    {
        auto d1 = q1 - p1;
        auto d2 = q2 - p2;
        auto r = p1 - p2;

        float a = d1 * d1;
        float e = d2 * d2;
        float f = d2 * r;
        float c = d1 * r;
        float b = d1 * d2;
        float denom = a * e - b * b;

        float t1, t2;
        if (fabsf(denom) < 0.0001f)
        {
            t1 = 0.0f;
            t2 = f / e;
        }
        else
        {
            t1 = (b * f - c * e) / denom;
            t2 = (b * t1 + f) / e;
        }
        t1 = std::max(0.0f, std::min(1.0f, t1));
        t2 = std::max(0.0f, std::min(1.0f, t2));
        auto closetA = p1 + d1 * t1;
        auto closetB = p2 + d2 * t2;
        return (closetA + closetB) * 0.5f;
    }
};