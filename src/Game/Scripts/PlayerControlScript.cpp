#include "PlayerControlScript.h"

#include "Engine/Core/GameObject/GameObject.h"
#include "Engine/Core/Components/TransformComponent.h"
#include "Engine/Core/Components/RigidbodyComponent.h"
#include "Engine/System/Input/InputManager.h"
#include "Engine/Core/GameWorld.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "Engine/Utils/JsonParser.h"

void PlayerControlScript::Initialize(const json &data)
{
    if (data.contains("Thrust"))
    {
        m_maxThrust = data["Thrust"];
    }
    if (data.contains("LiftCoefficient"))
    {
        m_liftCoefficient = data["LiftCoefficient"];
    }
    if (data.contains("DragCoefficient"))
    {
        m_dragCoefficient = data["DragCoefficient"];
    }
    if (data.contains("PitchPower"))
    {
        m_pitchPower = data["PitchPower"];
    }
    if (data.contains("YawPower"))
    {
        m_yawPower = data["YawPower"];
    }
    if (data.contains("RollPower"))
    {
        m_rollPower = data["RollPower"];
    }

    if (data.contains("ZoomSpeed"))
        m_zoomSpeed = data["ZoomSpeed"];
    if (data.contains("MinCamDist"))
        m_minCamDist = data["MinCamDist"];
    if (data.contains("MaxCamDist"))
        m_maxCamDist = data["MaxCamDist"];
}

void PlayerControlScript::OnCreate()
{

    auto &rb = owner->GetComponent<RigidbodyComponent>();
    Vector3f size = rb.localAABB.max - rb.localAABB.min;

    m_wingArea = size.x() * size.z();    // 机翼面积
    m_frontalArea = size.x() * size.y(); // 阻力面积
}
void PlayerControlScript::OnFixedUpdate(float dt)
{
    CalculatePhysics(dt);
}

void PlayerControlScript::CalculatePhysics(float dt)
{
    auto &rb = owner->GetComponent<RigidbodyComponent>();
    auto &tf = owner->GetComponent<TransformComponent>();
    auto &input = owner->GetOwnerWorld()->GetInputManager();

    Matrix4f worldMat = tf.GetWorldMatrix();
    Vector3f forward = tf.GetForward();
    Vector3f up = tf.GetUp();

    float airspeed = rb.velocity.Length();
    Vector3f velDir = (airspeed > 0.01f) ? rb.velocity.Normalized() : forward;

    // Angle of Attack - AoA
    float aoa = acosf(std::clamp(forward * velDir, -1.0f, 1.0f));

    // Lift:  L = 1/2 * p * v^2 * S * Cl
    float liftMag = airspeed * airspeed * m_wingArea * m_liftCoefficient;

    // 45度达到最大升力
    float liftFactor = sin(aoa * 2.0f);
    Vector3f liftDir = up;
    rb.AddForce(liftDir * (liftMag * liftFactor));

    // Drag
    float dragMag = airspeed * airspeed * m_frontalArea * m_dragCoefficient;
    float inducedDrag = liftMag * sin(aoa) * 0.5f;
    rb.AddForce((-velDir) * (dragMag + inducedDrag));

    if (input.IsActionDown("Thrust"))
    {
        rb.AddForce(forward * m_maxThrust);
        // 粒子
    }
    else if (input.IsActionDown("Brake"))
    {
        rb.AddForce((-forward) * (m_maxThrust * 0.5f));
    }

    float pitchInput = input.GetAxisValue("Pitch");
    float rollInput = input.GetAxisValue("Roll");
    float yawInput = input.GetAxisValue("Yaw");

    // 高速时更灵活，低速失控
    float speedFactor = std::clamp(airspeed / 50.0f, 0.0f, 2.0f);
    Vector3f localTorque = {
        pitchInput * m_pitchPower * speedFactor,
        yawInput * m_yawPower * speedFactor,
        rollInput * m_rollPower * speedFactor};
    Vector3f worldTorque = tf.GetWorldRotation() * (localTorque);
    rb.AddTorque(worldTorque);

    auto *camera = owner->GetOwnerWorld()->GetCameraManager().GetCamera("follow");

    camera->setFovy(60.0f * (1.0f + speedFactor));

    float scrollDelta = input.GetAxisValue("Scale");
    if (abs(scrollDelta) > 0.001f)
    {
        float zoomChange = (scrollDelta * m_zoomSpeed);
        m_camDistRatio += zoomChange;
        m_camDistRatio = std::clamp(m_camDistRatio, m_minCamDist / m_maxCamDist, 1.0f);
    }

    float baseDist = m_minCamDist + (m_maxCamDist - m_minCamDist) * m_camDistRatio;
    float finalDist = baseDist * (1.0f + speedFactor * 2.0f);

    Vector3f localPos = camera->getLocalPosition();
    Vector3f newPos = localPos.Normalized() * finalDist;
    Vector3f smoothedPos = Vector3f::Lerp(localPos, newPos, 0.1f);
    camera->setLocalPosition(smoothedPos);
}