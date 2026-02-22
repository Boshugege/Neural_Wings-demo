#pragma once
#include "Engine/System/HUD/IGameHud.h"
#include "Engine/Math/Math.h"
#include "raylib.h"
#include <string>

class GameWorld;
class mCamera;

class EntityPlateHud : public IGameHud
{
public:
    explicit EntityPlateHud(GameWorld *world);
    ~EntityPlateHud() override = default;

    void OnEnter() override;
    void FixedUpdate(float fixedDeltaTime) override;
    void Update(float deltaTime) override;
    void Draw() override;
    void OnExit() override;

private:
    bool TryProjectToScreen(const Vector3f &worldPos,
                            const mCamera &camera,
                            Vector2 &outScreenPos) const;
    void DrawNameplate(const std::string &displayName,
                       const Vector2 &screenPos) const;

    GameWorld *m_world = nullptr;
    float m_anchorHeight = 2.4f;
};
