#pragma once
#include "IComponent.h"
#include "raylib.h"
#include "Engine/Math/Math.h"
struct RenderComponent : public IComponent {
    Model model;
    Color tint = WHITE; // 模型颜色

    Vector3f scale = Vector3f(1.0f, 1.0f, 1.0f); // 渲染缩放
    // TODO：使用 ResourceManager 共享模型，不需要卸载。
    ~RenderComponent() {
        if (model.meshCount > 0) {
            UnloadModel(model);
        }
    }
};