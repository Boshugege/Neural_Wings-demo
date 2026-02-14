#pragma once

class ScreenManager;
class ScreenState;
class ResourceManager;
class AudioManager;

class IGameScreen
{
public:
    IGameScreen(ScreenManager *sm);
    virtual ~IGameScreen() = default;

    // 激活调用（初始化）
    virtual void OnEnter() = 0;

    // 固定时间步长调用（物理、网络）
    virtual void FixedUpdate(float fixedDeltaTime) = 0;

    // 每一帧都调用（输入、非物理逻辑）
    virtual void Update(float deltaTime) = 0;

    // Update 后（绘制）
    virtual void Draw() = 0;

    // 卸载资源、清理
    virtual void OnExit() = 0;

    // 状态
    virtual ScreenState GetNextScreenState() const = 0;
    virtual ScreenState GetScreenState() const = 0;

    ScreenManager *screenManager = nullptr;
    ResourceManager *resourceManager = nullptr;
    AudioManager *audioManager = nullptr;
};
