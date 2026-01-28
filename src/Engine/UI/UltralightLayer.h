#pragma once

#if !defined(PLATFORM_WEB)
#include "UILayer.h"
#include <Ultralight/Ultralight.h>
#include <Ultralight/Renderer.h>
#include <Ultralight/platform/Surface.h>

#include <raylib.h>

#include <cstdint>
#include <string>
#include <vector>

class UltralightLayer : public UILayer
{
public:
    UltralightLayer();
    virtual ~UltralightLayer();

    bool Initialize(uint32_t width, uint32_t height, const std::string &basePath) override;
    void Shutdown() override;

    void Update() override;
    void Draw() override;
    void HandleInput() override;

    void SetVisible(bool visible) override;
    bool IsVisible() const override;

    void LoadRoute(const std::string &route) override;
    void ExecuteScript(const std::string &script) override;

    std::string GetAppState(const std::string &key) const override;

    void Resize(uint32_t width, uint32_t height) override;
    std::string GetCurrentRoute() const override;

private:
    void EnsureTexture(uint32_t width, uint32_t height);
    void UpdateTextureFromView();
    std::string BuildIndexUrl(const std::string &route) const;

    ultralight::RefPtr<ultralight::Renderer> m_renderer;
    ultralight::RefPtr<ultralight::View> m_view;

    Texture2D m_texture;
    bool m_textureReady;
    bool m_visible;

    std::string m_basePath;
    std::vector<unsigned char> m_rgbaBuffer;
};
#endif