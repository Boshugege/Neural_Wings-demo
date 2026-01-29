#pragma once
#include "RenderView.h"
#include <vector>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
class GameWorld;
class CameraManager;

class Renderer
{
public:
    Renderer() = default;

    void AddRenderView(const RenderView &view);
    void ClearRenderViews();
    void RenderScene(const GameWorld &world, CameraManager &cameraManager);

    bool LoadViewConfig(const std::string &configPath);

private:
    RenderView ParseViews(const json &data);
    std::vector<RenderView> m_renderViews;

    void DrawWorldObjects(const GameWorld &world);
};