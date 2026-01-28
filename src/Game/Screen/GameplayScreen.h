#pragma once
#include "Engine/System/System.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/CameraManager.h"
#include "MyScreenState.h"

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
    ScreenState GetNextScreenState() const override;
    ScreenState GetScreenState() const override;

private:
    ScreenState m_nextScreenState;
    // 游戏世界系统
    std::unique_ptr<GameWorld> m_world;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<CameraManager> m_cameraManager;
    std::unique_ptr<InputManager> m_inputManager;
    std::unique_ptr<PhysicsSystem> m_physicsSystem;

    std::unique_ptr<ResourceManager> m_resourceManager;

    void ConfigureRenderer();
    // ...
};