#pragma once
#include "UILayer.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>

class WebLayer : public UILayer
{
public:
    WebLayer() : m_isVisible(true) {};
    bool Initialize(uint32_t width, uint32_t height, const std::string &basePath) override
    {
        ExecuteScript("window.dispatchEvent (new CustomEvent('Web Init!'));");
        return true;
    };
    void Shutdown() override {};

    void Update() override {};
    void Draw() override {};
    void HandleInput() override {};

    void SetVisible(bool visible) override
    {
        m_isVisible = visible;
        std::string display = visible ? "block" : "none";
        std::string js = "if(document.getElementById('app')) document.getElementById('app').style.display = '" + display + "';";
        ExecuteScript(js);
    };
    bool IsVisible() const override { return m_isVisible; };

    void LoadRoute(const std::string &route) override
    {
        std::string js;
        if (!route.empty() && route[0] == '#')
        {
            js = "window.location.hash = '" + route + "';";
        }
        else
        {
            js = "window.location.hash = '#/" + route + "';";
        }
        ExecuteScript(js);
    };

    void ExecuteScript(const std::string &script) override
    {
#if defined(PLATFORM_WEB)
        emscripten_run_script_string(script.c_str());
#endif
    }
    std::string GetAppState(const std::string &key) const override
    {
#if defined(PLATFORM_WEB)
        std::string js = "JSON.stringify(window.vueAppState && window.vueAppState." + key + ")";
        const char *result = emscripten_run_script_string(js.c_str());
        std::string resultStr = result ? result : "";
        if (resultStr.length() >= 2 && resultStr.front() == '"' && resultStr.back() == '"')
        {
            resultStr = resultStr.substr(1, resultStr.length() - 2);
        }
        return resultStr;
#else
        return "";
#endif
    };

    void Resize(uint32_t width, uint32_t height) override
    {
        ExecuteScript("window.dispatchEvent(new Event('resize'));");
    };
    std::string GetCurrentRoute() const override
    {
        const char *result = emscripten_run_script_string("window.location.hash");
        return result ? result : "";
    };

private:
    bool m_isVisible = true;
};
#endif
