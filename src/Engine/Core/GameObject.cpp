#include "GameObject.h"
#include <iostream>
unsigned int GameObject::s_nextID = 0;

GameObject::GameObject()
    : m_id(s_nextID++)
{
}
GameObject::~GameObject()
{
    std::cout << "Destroying GameObject " << m_id << std::endl;
}
unsigned int GameObject::GetID() const
{
    return m_id;
}
// 用于删除GameObject
void GameObject::Destroy()
{
    m_isWaitingDestroy = true;
}

bool GameObject::IsWaitingDestroy() const
{
    return m_isWaitingDestroy;
}