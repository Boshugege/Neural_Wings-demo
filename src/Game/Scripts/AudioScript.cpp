#include "AudioScript.h"
#include "Engine/Core/Components/Components.h"
#include "Engine/Core/GameObject/GameObject.h"
#include "Engine/Core/GameWorld.h"
void AudioScript::OnCreate()
{
    auto &audio = owner->GetComponent<AudioComponent>();
    audio.Play("Test_Velocity");
}
void AudioScript::OnUpdate(float deltaTime)
{
    auto &audio = owner->GetComponent<AudioComponent>();
    auto &input = owner->GetOwnerWorld()->GetInputManager();
    auto &rb = owner->GetComponent<RigidbodyComponent>();
    float speed = rb.velocity.Length();
    float targetPitch = 0.8f + (speed / 10.0f) * 0.7f;
    audio.SetPitch("Test_Velocity", targetPitch);
}