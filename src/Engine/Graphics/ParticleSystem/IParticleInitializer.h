#pragma once
#include "GPUParticle.h"
#include "Engine/Core/Components/TransformComponent.h"
#include <vector>

class IParticleInitializer
{
public:
    virtual ~IParticleInitializer() = default;
    virtual void Initialize(std::vector<GPUParticle> &particles, const TransformComponent &tf, int simSpace = 1) = 0;
};