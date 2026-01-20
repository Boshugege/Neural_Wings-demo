#pragma once
#include "Engine/System/Screen/GameScreen.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/CameraManager.h"

class GameplayScreen : public GameScreen
{
public:
    GameplayScreen();
    ~GameplayScreen();

    void OnEnter() override;
    void FixedUpdate(float fixedDeltaTime) override;
    void Update(float deltaTime) override;
    void Draw() override;
    void OnExit() override;
    int GetNextScreenState() const override;
    int GetScreenState() const override;
private:
    int m_nextScreenState;
    // 游戏世界系统
    std::unique_ptr<GameWorld> m_world;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<CameraManager> m_cameraManager;

    void ConfigureRenderer();
    // std::unique_ptr<PhysicsSystem> m_physicsSystem;
    // std::unique_ptr<UIManager> m_uiManager;
    // ...
};