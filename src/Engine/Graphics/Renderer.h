#pragma once
#include "RenderView.h"
#include "raylib.h"
#include "Camera/mCamera.h"
#include "RenderMaterial.h"
#include "Engine/Math/Math.h"
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
class GameWorld;
class CameraManager;
class Mesh;
class Model;

class Renderer
{
public:
    Renderer() = default;

    void AddRenderView(const RenderView &view);
    void ClearRenderViews();
    void RenderScene(GameWorld &world, CameraManager &cameraManager);

    bool LoadViewConfig(const std::string &configPath);

private:
    RenderView ParseViews(const json &data);
    std::vector<RenderView> m_renderViews;

    void RenderSinglePass(const Mesh &mesh, const Model &model, const int &meshIdx, const RenderMaterial &pass, const Matrix4f &MVP, const Matrix4f &M, const mCamera &camera, GameWorld &gameWorld);
    void DrawWorldObjects(GameWorld &gameWorld, Camera3D &rawCamera, mCamera &camera, float aspect);
};