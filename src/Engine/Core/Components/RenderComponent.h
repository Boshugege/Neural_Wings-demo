#pragma once
#include "IComponent.h"
#include "raylib.h"
#include "Engine/Math/Math.h"
#include "Engine/Graphics/RenderMaterial.h"
#include <unordered_map>

struct RenderComponent : public IComponent
{
    Model model;
    // RenderMaterial material;
    std::unordered_map<int, std::vector<RenderMaterial>> meshPasses;
    // 用于没有指派材质的mesh
    RenderMaterial defaultMaterial;

    bool isVisible = true;
    bool showWires = false;
    bool showAxes = false;
    bool showCenter = false;
    bool showAngVol = false;
    bool showVol = false;

    Vector3f scale = Vector3f(1.0f, 1.0f, 1.0f); // 渲染缩放
    bool castShadows = true;

    RenderComponent() = default;
    ~RenderComponent() = default;
};