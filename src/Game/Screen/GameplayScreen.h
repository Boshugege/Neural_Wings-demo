#pragma once
#include "Engine/System/System.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Graphics/Graphics.h"
#include "MyScreenState.h"

class GameplayScreen : public IGameScreen
{
public:
    GameplayScreen(ScreenManager *sm);
    ~GameplayScreen();

    void OnEnter() override;
    void FixedUpdate(float fixedDeltaTime) override;
    void Update(float deltaTime) override;
    void Draw() override;
    void OnExit() override;
    ScreenState GetNextScreenState() const override;
    ScreenState GetScreenState() const override;

private:
    ScreenState m_nextScreenState;
    // 游戏世界系统
    std::unique_ptr<GameWorld> m_world;
    void ConfigCallback(ScriptingFactory &scriptingFactory,
                        PhysicsStageFactory &physicsStageFactory,
                        ParticleFactory &particleFactory);
    // ...
};