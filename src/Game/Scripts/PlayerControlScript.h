#pragma once
#include "Engine/Core/Components/IScriptableComponent.h"
#include "Engine/Math/Math.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class PlayerControlScript : public IScriptableComponent
{
public:
    void Initialize(const json &data) override;
    void OnCreate() override;
    void OnFixedUpdate(float dt) override;

private:
    float m_maxThrust = 80.0f;      // 最大推力
    float m_liftCoefficient = 0.5f; // 升力系数
    float m_dragCoefficient = 0.1f; // 阻力系数

    // 机动灵敏
    float m_pitchPower = 30.0f;
    float m_rollPower = 50.0f;
    float m_yawPower = 15.0f;

    // 面积
    float m_wingArea = 1.0f;
    float m_frontalArea = 1.0f;

    void CalculatePhysics(float dt);

    float m_zoomSpeed = 2.0f;
    float m_minCamDist = 5.0f;
    float m_maxCamDist = 30.0f;
    float m_camDistRatio = 0.3f;
};