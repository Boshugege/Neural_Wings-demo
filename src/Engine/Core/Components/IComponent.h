#pragma once
class GameObject;
class IComponent
{
public:
    virtual ~IComponent() = default;
    GameObject *owner = nullptr;
};