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
// raylib源码修改，深度不再不可采样
#include "rlgl.h"
RenderTexture2D PostProcesser::LoadRT(int width, int height, PixelFormat format)
{
    RenderTexture2D target = {0};

    target.id = rlLoadFramebuffer(); // Load an empty framebuffer

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create color texture (default to RGBA)
        target.texture.id = rlLoadTexture(NULL, width, height, format, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = format;
        target.texture.mipmaps = 1;

        // Create depth renderbuffer/texture
        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19; // DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach color texture and depth renderbuffer/texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id))
            std::cout << "[PostProcesser]: [ID " << target.id << "] Framebuffer object created successfully" << std::endl;

        rlDisableFramebuffer();
    }

    return target;
}

void PostProcesser::DefaultSetup()
{
    std::vector<std::string> names = {"inScreen", "outScreen"};
    SetUpRTPool(names, GetScreenWidth(), GetScreenHeight());
}
void PostProcesser::SetUpRTPool(const std::vector<std::string> &names, int width, int height)
{
    UnloadRTPool();
    for (const auto &name : names)
    {
        std::string processName = name;
        const std::string &prefix = "accuracy_";
        // RenderTexture2D rt = LoadRenderTexture(width, height); 原raylib实现
        size_t pos = processName.find(prefix);
        PixelFormat format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        TextureFilter filter = TEXTURE_FILTER_BILINEAR;
        if (pos == 0)
        {
            processName = processName.substr(prefix.length());
            format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
        }
        RenderTexture2D rt = LoadRT(width, height, format);
        if (rt.id > 0)
        {
            m_RTPool[processName] = rt;
            SetTextureFilter(m_RTPool[processName].texture, filter);
            SetTextureFilter(m_RTPool[processName].depth, filter);
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
    m_fboDepthTracking.clear();
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
    if (data.contains("depthLinks"))
    {
        for (const auto &linkEntry : data["depthLinks"])
        {
            std::string src = linkEntry["source"];
            for (const std::string &dst : linkEntry["targets"])
            {
                LinkDepthBuffer(src, dst);
            }
        }
    }

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
        // TODO:支持动态纹理
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
#include "Engine/Graphics/Camera/CameraManager.h"
void PostProcesser::PostProcess(GameWorld &gameWorld, CameraManager &cameraManager)
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
                if (texUnit == 0)
                    firstInputId = m_RTPool[rtName].texture.id;
                mat.shader->SetTexture(shaderVarName, m_RTPool[rtName].texture, texUnit);
                // 仅rawScreen有深度
                if (rtName == "inScreen")
                {
                    mat.shader->SetTexture(shaderVarName + "_depth", m_RTPool[rtName].depth, texUnit + 1);
                    texUnit++;
                }
                texUnit++;
            }
        }
        // 外部纹理(无深度)
        for (auto const &[name, text] : mat.customTextures)
        {
            if (texUnit == 0)
                firstInputId = text.id;
            mat.shader->SetTexture(name, text, texUnit);

            // TODO:后处理加入动态纹理
            // mat.shader->SetTexture(name, text, texUnit);
            // if (mat.isAnimated.find(name) != mat.isAnimated.end())
            // {
            //     mat.shader->SetInt(name + "_frameCount", mat.frameCount.at(name));
            //     mat.shader->SetFloat(name + "_animSpeed", mat.animSpeed.at(name));
            // }
            // texUnit++;

            texUnit++;
        }
        // 上传参数
        mat.shader->SetFloat("gameTime", gameWorld.GetTimeManager().GetGameTime());
        mat.shader->SetFloat("realTime", gameWorld.GetTimeManager().GetRealTime());
        mat.shader->SetFloat("deltaRealTime", gameWorld.GetTimeManager().GetDeltaTime());
        mat.shader->SetFloat("deltaGameTime", gameWorld.GetTimeManager().GetFixedDeltaTime());

        mat.shader->SetVec3("cameraPosition", cameraManager.GetMainCamera()->Position());
        mat.shader->SetVec3("cameraDir", cameraManager.GetMainCamera()->Direction());
        mat.shader->SetVec3("cameraUp", cameraManager.GetMainCamera()->Up());
        mat.shader->SetVec3("cameraRight", cameraManager.GetMainCamera()->Right());
        mat.shader->SetFloat("cameraFov", cameraManager.GetMainCamera()->Fovy());
        mat.shader->SetFloat("cameraNear", cameraManager.GetMainCamera()->getNearPlane());
        mat.shader->SetFloat("cameraFar", cameraManager.GetMainCamera()->getFarPlane());

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
        mat.shader->End();
        EndTextureMode();
    }
    rlSetTexture(0);
}

void PostProcesser::LinkDepthBuffer(const std::string &sourceName, const std::string &targetName)
{
    auto itSrc = m_RTPool.find(sourceName);
    auto itDst = m_RTPool.find(targetName);
    if (itSrc == m_RTPool.end() || itDst == m_RTPool.end())
        return;

    RenderTexture2D &src = itSrc->second;
    RenderTexture2D &dst = itDst->second;
    if (m_fboDepthTracking.count(dst.id) && m_fboDepthTracking[dst.id] == src.depth.id)
    {
        return;
    }
    rlEnableFramebuffer(dst.id);
    rlFramebufferAttach(dst.id, src.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

    if (rlFramebufferComplete(dst.id))
    {
        m_fboDepthTracking[dst.id] = src.depth.id;
    }
    rlDisableFramebuffer();
}
void PostProcesser::UnlinkDepthBuffer(const std::string &targetName)
{
    auto itDst = m_RTPool.find(targetName);
    if (itDst == m_RTPool.end())
        return;
    RenderTexture2D &dst = itDst->second;
    if (m_fboDepthTracking.count(dst.id) && m_fboDepthTracking[dst.id] != dst.depth.id)
    {

        rlEnableFramebuffer(dst.id);
        rlFramebufferAttach(dst.id, dst.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);
        if (rlFramebufferComplete(dst.id))
        {
            m_fboDepthTracking[dst.id] = dst.depth.id;
        }
        rlDisableFramebuffer();
    }
}