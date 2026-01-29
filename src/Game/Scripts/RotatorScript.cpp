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
    tf.rotation = tf.rotation * Quat4f(m_angluarVelocity * fixedDeltaTime);
}
void RotatorScript::OnDestroy() {}