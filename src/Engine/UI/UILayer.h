#pragma once
#include <string>
#include <cstdint>

class UILayer
{
public:
    virtual ~UILayer() = default;

    virtual bool Initialize(uint32_t width, uint32_t height, const std::string &basePath) = 0;
    virtual void Shutdown() = 0;

    virtual void Update() = 0;
    virtual void Draw() = 0; // desktop纹理，web端空
    virtual void HandleInput() = 0;

    virtual void SetVisible(bool visible) = 0;
    virtual bool IsVisible() const = 0;

    virtual void LoadRoute(const std::string &route) = 0;
    virtual void ExecuteScript(const std::string &script) = 0;

    virtual std::string GetAppState(const std::string &key) const = 0;

    virtual void Resize(uint32_t width, uint32_t height) = 0;
    virtual std::string GetCurrentRoute() const = 0;
};