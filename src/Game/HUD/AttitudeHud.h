#pragma once
#include "Engine/System/HUD/IGameHud.h"
#include "raylib.h"

class GameWorld;
class GameObject;

class AttitudeHud : public IGameHud
{
public:
    explicit AttitudeHud(GameWorld *world);
    virtual ~AttitudeHud() override = default;

    void OnEnter() override {};
    void FixedUpdate(float fixedDeltaTime) override {};
    void Update(float deltaTime) override {};
    void Draw() override;
    void OnExit() override {};

private:
    GameObject *GetLocalPlayer() const;
    GameWorld *m_world = nullptr;

    void DrawPitchLine(float pitchDeg, float rollRad, float centerOffsetDeg,
                       float centerX, float centerY,
                       float width, Color color, float pixelsPerDegree) const;

    void DrawHorizonLine(float currentPitchDeg, float rollRad, float cx, float cy, float width, Color color, float pixelsPerDegree) const;
    const float m_pixelsPerDegree = 12.0f;
    const float m_ladderWidth = 200.0f;
    const float m_ladderGap = 10.0f;
    const int m_fovLimit = 20;
};