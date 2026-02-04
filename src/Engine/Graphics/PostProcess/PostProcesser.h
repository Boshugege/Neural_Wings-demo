#pragma once
#include "PostProcessPass.h"
#include "raylib.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class GameWorld;
class PostProcesser
{
public:
    void PostProcess(GameWorld &gameWorld);

    void ParsePostProcessPasses(const json &data, GameWorld &gameWorld);

    std::unordered_map<std::string, RenderTexture2D> &GetRTPool() { return m_RTPool; }

private:
    void DrawTextureQuad(float width, float height, bool flipY);
    void AddPostProcessPass(const PostProcessPass &pass);
    void SetUpRTPool(const std::vector<std::string> &names, int width, int height);
    void UnloadRTPool();

    RenderTexture2D PostProcesser::LoadRT(int width, int height);

    std::unordered_map<std::string, RenderTexture2D> m_RTPool;
    std::vector<PostProcessPass> m_postProcessPasses;
};