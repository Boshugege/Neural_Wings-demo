#include "Engine/Core/GameObject/GameObject.h"
#include "Engine/Core/Components/TransformComponent.h"

TransformComponent::TransformComponent(const Vector3f &pos) : localPosition(pos)
{
    isDirty = true;
}
TransformComponent::TransformComponent(const Vector3f &pos, const Quat4f &rot, const Vector3f &scl)
    : localPosition(pos), localRotation(rot), localScale(scl)
{
    isDirty = true;
}
void TransformComponent::SetOwner(GameObject *owner)
{
    this->owner = owner;
}
std::vector<GameObject *> TransformComponent::GetChildren() const
{
    return children;
}
Matrix4f TransformComponent::GetLocalMatrix() const
{
    Matrix4f S = Matrix4f(Matrix3f(localScale));
    Matrix4f R = Matrix4f(localRotation.toMatrix());
    Matrix4f T = Matrix4f::translation(localPosition);
    Matrix4f M = T * R * S;
    return M;
}

Matrix4f TransformComponent::GetWorldMatrix() const
{
    return worldMatrix;
}
void TransformComponent::SetWorldMatrix(const Matrix4f &newWorldMat)
{
    worldMatrix = newWorldMat;
    Matrix4f localMat;

    if (parent == nullptr)
        localMat = newWorldMat;
    else
    {
        Matrix4f invParent = parent->GetComponent<TransformComponent>().worldMatrix.inverse();
        localMat = invParent * newWorldMat;
    }
    this->localPosition = localMat.getTranslation();
    this->localRotation = localMat.getRotation();
    this->localScale = localMat.getScale();
    this->isDirty = false;
    for (auto *child : children)
    {
        if (child)
            child->GetComponent<TransformComponent>().SetDirty();
    }
}
void TransformComponent::SetLocalPosition(const Vector3f &pos)
{
    localPosition = pos;
    SetDirty();
}
Vector3f TransformComponent::GetLocalPosition() const { return localPosition; }
void TransformComponent::SetLocalRotation(const Quat4f &rot)
{
    localRotation = rot;
    SetDirty();
}
Quat4f TransformComponent::GetLocalRotation() const { return localRotation; }
void TransformComponent::SetLocalScale(const Vector3f &scl)
{
    localScale = scl;
    SetDirty();
}
Vector3f TransformComponent::GetLocalScale() const { return localScale; }

void TransformComponent::RemoveChild(GameObject *child)
{
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end())
        children.erase(it);
}
void TransformComponent::SetParent(GameObject *newParent)
{
    if (newParent == this->parent)
        return;
    if (this->parent != nullptr)
    {
        this->parent->GetComponent<TransformComponent>().RemoveChild(owner);
    }
    this->parent = newParent;
    if (this->parent != nullptr)
    {
        parent->GetComponent<TransformComponent>().children.push_back(owner);
    }
    SetDirty();
}
GameObject *TransformComponent::GetParent() const { return parent; }

void TransformComponent::SetDirty()
{
    if (isDirty)
        return;
    isDirty = true;
    for (auto *child : children)
    {
        child->GetComponent<TransformComponent>().SetDirty();
    }
}

void TransformComponent::SetClean()
{
    isDirty = false;
}
Vector3f TransformComponent::GetWorldPosition() const { return worldMatrix.getTranslation(); }
Quat4f TransformComponent::GetWorldRotation() const { return worldMatrix.getRotation(); }
Vector3f TransformComponent::GetWorldScale() const { return worldMatrix.getScale(); }
Vector3f TransformComponent::GetForward() const { return worldMatrix.getCol(2).xyz().Normalized(); }