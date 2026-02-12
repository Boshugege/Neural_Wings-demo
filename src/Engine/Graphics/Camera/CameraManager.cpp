#include "CameraManager.h"
#include "raylib.h"
#include "raymath.h"
#include <fstream>
#include <iostream>
#include "Engine/Utils/JsonParser.h"
#include <nlohmann/json.hpp>
#include "Engine/Core/GameWorld.h"
using json = nlohmann::json;

mCamera &CameraManager::CreateCamera(const std::string &name)
{
    return m_cameras[name];
}

bool CameraManager::CreateCameraFromConfig(const json &configData)
{
    if (!configData.contains("name"))
    {
        std::cerr << "[CameraManager]: Missing 'name' field in camera config" << std::endl;
        return false;
    }

    std::string name = configData["name"];
    auto &cam = m_cameras[name];
    cam.setPosition(Vector3f(configData.value("position", json::array({0.f, 0.f, 0.f}))));
    cam.setTarget(Vector3f(configData.value("target", json::array({0.f, 0.f, 0.f}))));
    cam.setDirection((cam.Target() - cam.Position()).Normalized());
    cam.setUp(Vector3f(configData.value("up", json::array({0.f, 1.f, 0.f}))));
    cam.setRight(cam.Direction() ^ cam.Up());
    cam.setFovy(configData.value("fovy", 60.0f));
    cam.setNearPlane(configData.value("nearPlane", 0.1f));
    cam.setFarPlane(configData.value("farPlane", 1000.0f));

    std::string proj = configData.value("projection", "PERSPECTIVE");
    if (proj == "ORTHOGRAPHIC")
    {
        // cam.projection = CAMERA_ORTHOGRAPHIC;
        cam.setProjection(CAMERA_ORTHOGRAPHIC);
    }
    else
    {
        // cam.projection = CAMERA_PERSPECTIVE;
        cam.setProjection(CAMERA_PERSPECTIVE);
    }

    if (configData.value("isMain", false))
    {
        SetMainCamera(name);
    }

    if (configData.contains("mountTarget"))
    {
        std::string targetName = configData["mountTarget"];
        Vector3f posOffset = JsonParser::ToVector3f(configData["posOffset"]);
        Vector3f lookAtOffset = JsonParser::ToVector3f(configData["lookAtOffset"]);
        cam.SetMountIntent(targetName, posOffset, lookAtOffset);
    }
    return true;
}
void CameraManager::ResolveMounts(GameWorld &world)
{
    for (auto &pair : m_cameras)
    {
        mCamera &cam = pair.second;
        std::string targetName = cam.GetMountTargetName();
        if (!targetName.empty())
        {
            GameObject *targetObj = world.FindEntityByName(targetName);
            if (targetObj)
            {
                cam.SetMountTarget(targetObj);
                std::cout << "[CameraManager]: Camera '" << pair.first
                          << "' mounted to '" << targetName << "'" << std::endl;
            }
        }
    }
}

bool CameraManager::RemoveCamera(const std::string &name)
{
    if (name == m_mainCameraName)
    {
        m_mainCameraName.clear();
    }
    return m_cameras.erase(name) > 0;
}

bool CameraManager::LoadConfig(const std::string &filePath)
{
    m_cameras.clear();
    m_mainCameraName.clear();

    std::ifstream configFile(filePath);
    if (!configFile.is_open())
    {
        std::cerr << "Error: [CameraManager] Could not open file: " << filePath << std::endl;
        return false;
    }

    json configJson;
    try
    {
        configFile >> configJson;
        if (configJson.contains("cameras") && configJson["cameras"].is_array())
        {
            for (const auto &camConfig : configJson["cameras"])
            {
                CreateCameraFromConfig(camConfig);
            }
        }
    }
    catch (json::parse_error &e)
    {
        std::cerr << "Error: [CameraManager] Failed to parse " << filePath << ": " << e.what() << std::endl;
        return false;
    }

    // 若没有主相机，设置第一个为默认主相机
    if (m_mainCameraName.empty() && !m_cameras.empty())
    {
        SetMainCamera(m_cameras.begin()->first);
    }

    return true;
}

mCamera *CameraManager::GetCamera(const std::string &name)
{
    auto it = m_cameras.find(name);
    if (it != m_cameras.end())
    {
        return &(it->second);
    }
    return nullptr;
}

mCamera *CameraManager::GetMainCamera()
{
    if (m_mainCameraName.empty())
        return nullptr;
    return GetCamera(m_mainCameraName);
}

void CameraManager::SetMainCamera(const std::string &name)
{
    if (m_cameras.count(name))
    {
        m_mainCameraName = name;
    }
}