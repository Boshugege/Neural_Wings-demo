#include "HudManager.h"
#include <algorithm>
#include <utility>

HudManager::HudManager(std::unique_ptr<HudFactory> factory)
    : m_factory(std::move(factory))
{
}

HudManager::~HudManager()
{
    Clear();
}

void HudManager::SetFactory(std::unique_ptr<HudFactory> factory)
{
    Clear();
    m_factory = std::move(factory);
}

bool HudManager::AddHud(int state)
{
    if (HasHud(state))
        return true;
    if (!m_factory)
        return false;

    auto hud = m_factory->Create(state);
    if (!hud)
        return false;

    hud->OnEnter();
    m_order.push_back(state);
    m_huds[state] = std::move(hud);
    return true;
}

void HudManager::RemoveHud(int state)
{
    auto it = m_huds.find(state);
    if (it == m_huds.end())
        return;

    it->second->OnExit();
    m_huds.erase(it);
    m_order.erase(std::remove(m_order.begin(), m_order.end(), state), m_order.end());
}

bool HudManager::HasHud(int state) const
{
    return m_huds.find(state) != m_huds.end();
}

void HudManager::Clear()
{
    for (int state : m_order)
    {
        auto it = m_huds.find(state);
        if (it != m_huds.end() && it->second)
        {
            it->second->OnExit();
        }
    }
    m_huds.clear();
    m_order.clear();
}

void HudManager::FixedUpdate(float fixedDeltaTime)
{
    for (int state : m_order)
    {
        auto it = m_huds.find(state);
        if (it != m_huds.end() && it->second)
            it->second->FixedUpdate(fixedDeltaTime);
    }
}

void HudManager::Update(float deltaTime)
{
    for (int state : m_order)
    {
        auto it = m_huds.find(state);
        if (it != m_huds.end() && it->second)
            it->second->Update(deltaTime);
    }
}

void HudManager::Draw()
{
    for (int state : m_order)
    {
        auto it = m_huds.find(state);
        if (it != m_huds.end() && it->second)
            it->second->Draw();
    }
}

bool HudManager::BlocksGameplayInput() const
{
    for (int state : m_order)
    {
        auto it = m_huds.find(state);
        if (it != m_huds.end() && it->second && it->second->BlocksGameplayInput())
            return true;
    }
    return false;
}

bool HudManager::WantsCursorVisible() const
{
    for (int state : m_order)
    {
        auto it = m_huds.find(state);
        if (it != m_huds.end() && it->second && it->second->WantsCursorVisible())
            return true;
    }
    return false;
}

bool HudManager::ConsumeExitSuppressRequest()
{
    bool consumed = false;
    for (int state : m_order)
    {
        auto it = m_huds.find(state);
        if (it != m_huds.end() && it->second && it->second->ConsumeExitSuppressRequest())
            consumed = true;
    }
    return consumed;
}
