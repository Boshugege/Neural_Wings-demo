#include "RenderViewer.h"
#include <iostream>
#include <string>
#include <vector>
#include "Engine/Utils/JsonParser.h"

std::vector<RenderView> &RenderViewer::GetRenderViews()
{
    return m_renderViews;
}
void RenderViewer::ParseViewConfig(const json &data)
{
    this->ClearRenderViews();
    for (const auto &viewData : data)
    {
        RenderView view = ParseViews(viewData);
        this->AddRenderView(view);
    }
}
void RenderViewer::ClearRenderViews()
{
    m_renderViews.clear();
}
void RenderViewer::AddRenderView(const RenderView &view)
{
    m_renderViews.push_back(view);
}

RenderView RenderViewer::ParseViews(const json &viewData)
{

    RenderView view;
    if (viewData.contains("name"))
        // 与camerajson中名字一致
        view.cameraName = viewData["name"];
    else
    {
        std::cerr << "[Renderer]: View config file missing 'name' field" << std::endl;
        return view;
    }
    if (viewData.contains("viewport"))
        view.viewport = JsonParser::ToRectangle(viewData["viewport"]);
    else
        view.viewport = Rectangle{0.0, 0.0, (float)GetScreenHeight(), (float)GetScreenWidth()};

    view.clearBackground = viewData.value("clearBackground", false);
    if (viewData.contains("backgroundColor"))
        view.backgroundColor = JsonParser::ToColor(viewData["backgroundColor"]);
    else
        view.backgroundColor = WHITE;
    return view;
}