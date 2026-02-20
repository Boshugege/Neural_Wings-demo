#pragma once
#include "Engine/Graphics/ShaderWrapper.h"
#include "Engine/Core/Components/Components.h"
#include <vector>

#define MAX_LIGHTS 16
#define MAX_SHADOW_CASTERS 6
#define MAX_POINT_SHADOWS 6

class GameWorld;
class LightingManager
{
public:
    ~LightingManager();
    void Update(GameWorld &world);
    void UploadToShader(std::shared_ptr<ShaderWrapper> shader, const Vector3f &viewPos, int texUnit);

    void InitShadowMaps(int width, int height, ResourceManager &rm);
    void RenderShadowMaps(GameWorld &world, const Vector3f &centerPos);

    RenderTexture2D *GetShadowMap(int index);

private:
    struct ShadowCasterData
    {
        LightComponent *light;
        GameObject *owner;
        int textureIndex;
        Matrix4f lightVP;
        Vector3f lightDir;
    };
    std::vector<RenderTexture2D> m_shadowMaps;
    std::shared_ptr<ShaderWrapper> m_depthShader;

    struct PointShadowMap
    {
        unsigned int fboId = 0;
        unsigned int cubemapId = 0;
        int resolution = 1024;
    };
    std::vector<PointShadowMap> m_pointShadowMaps;
    std::shared_ptr<ShaderWrapper> m_pointDepthShader;

    struct LightInfo
    {
        LightComponent *data;
        Vector3f worldPosition;
        Vector3f worldDirection;
        int shadowIndex = -1;
    };

    std::vector<LightInfo> m_activeLights;

    std::vector<ShadowCasterData> m_activeCasters;
    std::vector<ShadowCasterData> m_activePointCasters;

    Matrix4f CalculateDirectionalLightVP(const Vector3f &lightDir, const Vector3f &centerPos);
};