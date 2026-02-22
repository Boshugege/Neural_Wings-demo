#pragma once

class IGameHud
{
public:
    virtual ~IGameHud() = default;

    virtual void OnEnter() = 0;
    virtual void FixedUpdate(float fixedDeltaTime) = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
    virtual void OnExit() = 0;

    // If true, gameplay-level input (for flight/control) should be considered blocked.
    virtual bool BlocksGameplayInput() const { return false; }

    // If true, cursor should be visible while this HUD is active.
    virtual bool WantsCursorVisible() const { return false; }

    // Used by HUDs that need to consume "Exit" style input in current frame.
    virtual bool ConsumeExitSuppressRequest() { return false; }
};
