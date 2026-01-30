#pragma once
#include "Engine/Core/Components/Components.h"
#include <vector>
#include <memory>
#include <typeindex>
#include <stdexcept>
#include <string>

#include "raylib.h"
#include "Engine/Math/Math.h"

class GameWorld;
struct AABB;
class GameObject
{
public:
    GameObject(unsigned int id, std::string name = "", std::string tag = "Untagged");
    ~GameObject();

    GameObject(const GameObject &) = delete;
    GameObject &operator=(const GameObject &) = delete;

    void OnDestroy();
    void SetIsWaitingDestroy(bool isWaitingDestroy);
    bool IsWaitingDestroy() const;

    template <typename T, typename... Args>
    T &AddComponent(Args &&...args);
    template <typename T>
    T &GetComponent() const;
    template <typename T>
    bool HasComponent() const;

    unsigned int GetID() const;
    AABB GetWorldAABB(Vector3f (*outCorners)[8] = nullptr) const;

    void SetName(const std::string &name);
    void SetTag(const std::string &tag);
    std::string GetName() const;
    std::string GetTag() const;
    GameWorld *GetOwnerWorld() const;
    void SetOwnerWorld(GameWorld *world);

private:
    GameWorld *owner_world = nullptr;

    std::string m_name;
    std::string m_tag;

    std::vector<std::unique_ptr<IComponent>> m_components;
    // 组件索引
    std::vector<std::type_index> m_componentTypeIndex;

    const unsigned int m_id;
    static unsigned int s_nextID;

    bool m_isWaitingDestroy = false;
};

template <typename T, typename... TArgs>
T &GameObject::AddComponent(TArgs &&...args)
{
    if (HasComponent<T>())
    {
        throw std::runtime_error("Component already exists.");
    }

    auto newComponent = std::make_unique<T>(std::forward<TArgs>(args)...);
    T *rawPtr = newComponent.get();

    m_components.push_back(std::move(newComponent));
    m_componentTypeIndex.push_back(std::type_index(typeid(T)));

    return *rawPtr;
}

template <typename T>
T &GameObject::GetComponent() const
{
    auto targetType = std::type_index(typeid(T));
    for (size_t i = 0; i < m_componentTypeIndex.size(); ++i)
    {
        if (m_componentTypeIndex[i] == targetType)
        {
            return *dynamic_cast<T *>(m_components[i].get());
        }
    }
    throw std::runtime_error("Component not found!");
}

template <typename T>
bool GameObject::HasComponent() const
{
    auto targetType = std::type_index(typeid(T));
    for (const auto &type : m_componentTypeIndex)
    {
        if (type == targetType)
        {
            return true;
        }
    }
    return false;
}
