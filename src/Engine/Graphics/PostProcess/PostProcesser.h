#pragma once
#include "PostProcessPass.h"
#include "raylib.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class GameWorld;
class CameraManager;
class PostProcesser
{
public:
    void PostProcess(GameWorld &gameWorld, CameraManager &cameraManager);

    void ParsePostProcessPasses(const json &data, GameWorld &gameWorld);
    std::unordered_map<std::string, RenderTexture2D> &GetRTPool() { return m_RTPool; }
    void DefaultSetup();

    void LinkDepthBuffer(const std::string &sourceName, const std::string &targetName);
    void UnlinkDepthBuffer(const std::string &targetName);

private:
    void DrawTextureQuad(float width, float height, bool flipY);
    void AddPostProcessPass(const PostProcessPass &pass);
    void SetUpRTPool(const std::vector<std::string> &names, int width, int height);
    void UnloadRTPool();

    RenderTexture2D PostProcesser::LoadRT(int width, int height, PixelFormat format);

    std::unordered_map<std::string, RenderTexture2D> m_RTPool;
    std::vector<PostProcessPass> m_postProcessPasses;

    std::unordered_map<unsigned int, unsigned int> m_fboDepthTracking;
};