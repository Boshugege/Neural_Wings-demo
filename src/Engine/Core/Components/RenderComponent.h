#pragma once
#include "IComponent.h"
#include "raylib.h"

struct RenderComponent : public IComponent {
    Model model;
    Color tint = WHITE; // 模型颜色

    // TODO：使用 ResourceManager 共享模型，不需要卸载。
    ~RenderComponent() {
        if (model.meshCount > 0) {
            UnloadModel(model);
        }
    }
};