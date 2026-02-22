#pragma once
#include "HudFactory.h"
#include <memory>
#include <unordered_map>
#include <vector>

class HudManager
{
public:
    explicit HudManager(std::unique_ptr<HudFactory> factory = nullptr);
    ~HudManager();

    void SetFactory(std::unique_ptr<HudFactory> factory);

    bool AddHud(int state);
    void RemoveHud(int state);
    bool HasHud(int state) const;
    void Clear();

    void FixedUpdate(float fixedDeltaTime);
    void Update(float deltaTime);
    void Draw();

    bool BlocksGameplayInput() const;
    bool WantsCursorVisible() const;
    bool ConsumeExitSuppressRequest();

private:
    std::unique_ptr<HudFactory> m_factory;
    std::unordered_map<int, std::unique_ptr<IGameHud>> m_huds;
    std::vector<int> m_order;
};
