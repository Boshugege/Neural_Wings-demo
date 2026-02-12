#pragma once
#include "IComponent.h"
#include "raylib.h"
#include "Engine/Math/Math.h"
class GameObject;

class TransformComponent : public IComponent
{
private:
    Vector3f localPosition = Vector3f(0.0f, 0.0f, 0.0f);
    Quat4f localRotation = Quat4f(1.0f, 0.0f, 0.0f, 0.0f);
    Vector3f localScale = Vector3f(1.0f, 1.0f, 1.0f);

    Matrix4f worldMatrix = Matrix4f::identity();

    GameObject *owner = nullptr;        // 组件所属对象
    GameObject *parent = nullptr;       // 父对象
    std::vector<GameObject *> children; // 子对象

public:
    bool isDirty = true;

    TransformComponent() = default;
    TransformComponent(const Vector3f &pos);

    TransformComponent(const Vector3f &pos, const Quat4f &rot, const Vector3f &scl);

    std::vector<GameObject *> GetChildren() const;
    void SetOwner(GameObject *owner);

    Matrix4f GetLocalMatrix() const;
    Matrix4f GetWorldMatrix() const;
    void SetWorldMatrix(const Matrix4f &mat);

    void SetLocalPosition(const Vector3f &pos);
    Vector3f GetLocalPosition() const;

    void SetLocalRotation(const Quat4f &rot);
    Quat4f GetLocalRotation() const;

    void SetLocalScale(const Vector3f &scl);
    Vector3f GetLocalScale() const;

    void RemoveChild(GameObject *child);
    void SetParent(GameObject *newParent);
    GameObject *GetParent() const;

    void SetDirty();
    void SetClean();

    Vector3f GetWorldPosition() const;
    Quat4f GetWorldRotation() const;
    Vector3f GetWorldScale() const;
    Vector3f GetForward() const;
};