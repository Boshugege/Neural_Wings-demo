#include "HudFactory.h"
#include <utility>

void HudFactory::Register(int state, HudCreator creator)
{
    m_creators[state] = std::move(creator);
}

std::unique_ptr<IGameHud> HudFactory::Create(int state) const
{
    const auto it = m_creators.find(state);
    if (it == m_creators.end())
        return nullptr;

    return (it->second)();
}
