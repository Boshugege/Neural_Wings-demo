#include "RotatorScript.h"
#include "Engine/Math/Math.h"
#include "Engine/Utils/JsonParser.h"
#include "Engine/Core/Components/Components.h"
#include "Engine/Core/GameObject/GameObject.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

void RotatorScript::Initialize(const json &data)
{
    if (data.contains("angluarVelocity"))
    {
        m_angluarVelocity = JsonParser::ToVector3f(data["angluarVelocity"]);
    }
}

void RotatorScript::OnFixedUpdate(float fixedDeltaTime)
{
    auto &tf = owner->GetComponent<TransformComponent>();
    Quat4f rot = tf.GetWorldRotation();
    Vector3f scale = tf.GetWorldScale();
    Vector3f pos = tf.GetWorldPosition();

    rot = (rot * Quat4f(m_angluarVelocity * fixedDeltaTime));
    tf.SetWorldMatrix(Matrix4f::CreateTransform(pos, rot, scale));
}
void RotatorScript::OnDestroy() {}