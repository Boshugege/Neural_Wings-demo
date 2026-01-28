#pragma once

#include <Ultralight/Ultralight.h>
#include <Ultralight/Renderer.h>
#include <Ultralight/platform/Surface.h>

#include <raylib.h>

#include <cstdint>
#include <string>
#include <vector>

class UltralightLayer
{
public:
    UltralightLayer();
    ~UltralightLayer();

    bool Initialize(uint32_t width, uint32_t height, const std::string &basePath);
    void Shutdown();

    void SetVisible(bool visible);
    bool IsVisible() const;

    void LoadRoute(const std::string &route);
    void Resize(uint32_t width, uint32_t height);

    // Get the current route from Vue app state
    std::string GetCurrentRoute() const;

    // Get Vue app state (settings)
    std::string GetAppState(const std::string &key) const;

    // Execute JavaScript code
    void ExecuteScript(const std::string &script);

    void Update();
    void HandleInput();
    void Draw();

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
