#include "NetworkVerifyStage.h"
#include "Engine/Core/GameWorld.h"

void NetworkVerifyStage::Initialize(const json & /*config*/)
{
}

void NetworkVerifyStage::Execute(GameWorld & /*world*/, float /*fixedDeltaTime*/)
{
    // Keep physics stage chain valid while isolating network sync verification.
}
