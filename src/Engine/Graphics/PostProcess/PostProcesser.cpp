#include "PostProcesser.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Utils/JsonParser.h"
void PostProcesser::AddPostProcessPass(const PostProcessPass &pass)
{
    if (pass.outputTarget.empty())
    {
        std::cerr << "[PostProcesser]: Post process pass missing output target: " << pass.name << std::endl;
        return;
    }
    m_postProcessPasses.push_back(pass);
    std::cout << "[PostProcesser]: Post process pass added: " << pass.name << " output target -> " << pass.outputTarget << std::endl;
}
void PostProcesser::SetUpRTPool(const std::vector<std::string> &names, int width, int height)
{
    UnloadRTPool();
    for (const auto &name : names)
    {
        RenderTexture2D rt = LoadRenderTexture(width, height);
        if (rt.id > 0)
        {
            m_RTPool[name] = rt;
            SetTextureFilter(m_RTPool[name].texture, TEXTURE_FILTER_BILINEAR);
        }
        else
        {
            std::cerr << "[PostProcesser]: Failed to create render texture: " << name << std::endl;
        }
    }
    std::cout << "[PostProcesser]: Render texture pool set up with " << names.size() << " render targets" << std::endl;
}
void PostProcesser::UnloadRTPool()
{
    int count = 0;

    for (auto &pair : m_RTPool)
    {
        UnloadRenderTexture(pair.second);
        count++;
    }
    m_RTPool.clear();
    m_postProcessPasses.clear();
    std::cout << "[PostProcesser]: Unloaded " << count << " render targets" << std::endl;
}

void PostProcesser::ParsePostProcessPasses(const json &data, GameWorld &gameWorld)
{
    if (!data.contains("rtPool"))
    {
        std::cerr << "[PostProcesser]: Post process config file missing 'rtPool' field" << std::endl;
        return;
    }
    if (!data.contains("postProcessGraph"))
    {
        std::cerr << "[PostProcesser]: Post process config file missing 'postProcessGraph' field" << std::endl;
        return;
    }
    const std::vector<std::string> &rtNames = data["rtPool"].get<std::vector<std::string>>();
    this->SetUpRTPool(rtNames, GetScreenWidth(), GetScreenHeight());
    this->m_postProcessPasses.clear();
    auto &rm = gameWorld.GetResourceManager();

    for (const auto &passData : data["postProcessGraph"])
    {
        PostProcessPass pass;
        pass.name = passData["name"];

        // 解析输入输出
        pass.outputTarget = passData["output"];
        if (passData.contains("inputs"))
            for (const auto &inputItem : passData["inputs"])
            {
                if (inputItem.size() == 2)
                    pass.inputs.push_back({inputItem[0], inputItem[1]});
                else
                    std::cerr << "[PostProcesser]: Post process pass: " << pass.name << " input item size not equal to 2" << std::endl;
            }
        // 解析shader
        PostRenderMaterial &mat = pass.material;
        mat.shader = rm.GetShader(
            passData.value("vs", "assets/shaders/postprocess/default.vs"),
            passData.value("fs", "assets/shaders/postprocess/default.fs"));
        if (passData.contains("baseColor"))
            mat.baseColor = JsonParser::ToVector4f(passData["baseColor"]);
        if (passData.contains("uniforms"))
        {
            auto &uniformsData = passData["uniforms"];
            for (auto &[uName, uValue] : uniformsData.items())
            {
                if (uValue.is_number())
                    mat.customFloats[uName] = uValue;
                else if (uValue.is_array() && uValue.size() == 2)
                    mat.customVector2[uName] = JsonParser::ToVector2f(uValue);
                else if (uValue.is_array() && uValue.size() == 3)
                    mat.customVector3[uName] = JsonParser::ToVector3f(uValue);
                else if (uValue.is_array() && uValue.size() == 4)
                    mat.customVector4[uName] = JsonParser::ToVector4f(uValue);
                else
                    std::cerr << "[PostProcesser]:ParsePostProcessPasses Unknown uniform type: " << uName << std::endl;
            }
        }
        // 解析外部纹理
        if (passData.contains("textures"))
        {
            auto &texData = passData["textures"];
            for (auto &[texName, texPath] : texData.items())
            {
                Texture2D tex = rm.GetTexture2D(texPath);
                if (tex.id > 0)
                    mat.customTextures[texName] = tex;
            }
        }
        this->AddPostProcessPass(pass);
    }
}
#include "rlgl.h"
void PostProcesser::DrawTextureQuad(float width, float height, bool flipY)
{
    rlBegin(RL_QUADS);
    rlColor4ub(255, 255, 255, 255);
    if (flipY)
    {
        rlTexCoord2f(0.0f, 1.0f);
        rlVertex2f(0.0f, 0.0f);
        rlTexCoord2f(1.0f, 1.0f);
        rlVertex2f(width, 0.0f);
        rlTexCoord2f(1.0f, 0.0f);
        rlVertex2f(width, height);
        rlTexCoord2f(0.0f, 0.0f);
        rlVertex2f(0.0f, height);
    }
    else
    {
        rlTexCoord2f(0.0f, 0.0f);
        rlVertex2f(0.0f, 0.0f);
        rlTexCoord2f(1.0f, 0.0f);
        rlVertex2f(width, 0.0f);
        rlTexCoord2f(1.0f, 1.0f);
        rlVertex2f(width, height);
        rlTexCoord2f(0.0f, 1.0f);
        rlVertex2f(0.0f, height);
    }
    rlEnd();
}

void PostProcesser::PostProcess(GameWorld &gameWorld)
{
    for (auto &pass : m_postProcessPasses)
    {
        auto &itOut = m_RTPool.find(pass.outputTarget);
        if (itOut == m_RTPool.end())
            continue;

        BeginTextureMode(itOut->second);
        ClearBackground(BLANK);

        auto &mat = pass.material;
        mat.shader->Begin();

        // 上传输入纹理
        int texUnit = 0;
        unsigned int firstInputId = 0;
        for (const auto &[shaderVarName, rtName] : pass.inputs)
        {
            if (m_RTPool.count(rtName))
            {
                mat.shader->SetTexture(shaderVarName, m_RTPool[rtName].texture, texUnit);
                if (texUnit == 0)
                    firstInputId = m_RTPool[rtName].texture.id;
                texUnit++;
            }
        }
        // 外部纹理
        for (auto const &[name, text] : mat.customTextures)
        {
            if (texUnit == 0)
                firstInputId = text.id;
            mat.shader->SetTexture(name, text, texUnit);
            texUnit++;
        }
        // 上传参数
        mat.shader->SetFloat("gameTime", gameWorld.GetTimeManager().GetGameTime());
        mat.shader->SetFloat("realTime", gameWorld.GetTimeManager().GetRealTime());
        mat.shader->SetFloat("deltaRealTime", gameWorld.GetTimeManager().GetDeltaTime());
        mat.shader->SetFloat("deltaGameTime", gameWorld.GetTimeManager().GetFixedDeltaTime());

        Vector2f screenRes((float)itOut->second.texture.width, (float)itOut->second.texture.height);
        mat.shader->SetVec2("screenResolution", screenRes);

        mat.shader->SetVec4("baseColor", mat.baseColor);

        for (auto const &[name, value] : mat.customFloats)
            mat.shader->SetFloat(name, value);
        for (auto const &[name, value] : mat.customVector2)
            mat.shader->SetVec2(name, value);
        for (auto const &[name, value] : mat.customVector3)
            mat.shader->SetVec3(name, value);
        for (auto const &[name, value] : mat.customVector4)
            mat.shader->SetVec4(name, value);

        rlSetTexture(firstInputId);
        DrawTextureQuad(screenRes.x(), screenRes.y(), true);
        rlSetTexture(0);
        mat.shader->End();
        EndTextureMode();
    }
}