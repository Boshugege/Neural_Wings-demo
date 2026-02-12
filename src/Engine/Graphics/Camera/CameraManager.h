#pragma once
#include "raylib.h"
#include "mCamera.h"
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
class GameWorld;

class CameraManager
{
public:
    CameraManager() = default;
    mCamera &CreateCamera(const std::string &name); // 默认相机
    bool CreateCameraFromConfig(const json &configData);
    void ResolveMounts(GameWorld &world);

    bool RemoveCamera(const std::string &name);
    bool LoadConfig(const std::string &filePath); // 加载多相机

    mCamera *GetCamera(const std::string &name);
    mCamera *GetMainCamera();
    void SetMainCamera(const std::string &name);

private:
    std::unordered_map<std::string, mCamera> m_cameras;
    std::string m_mainCameraName;
};