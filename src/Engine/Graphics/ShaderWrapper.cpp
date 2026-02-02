#include "ShaderWrapper.h"

#include <iostream>

ShaderWrapper::ShaderWrapper(const std::string &vsPath, const std::string &fsPath)
{
    const char *vPath = vsPath.empty() ? nullptr : vsPath.c_str();
    const char *fPath = fsPath.empty() ? nullptr : fsPath.c_str();

    m_shader = LoadShader(vPath, fPath);

    if (m_shader.id <= 0)
    {
        std::cerr << "[ShaderWrapper] Failed to load shader: " << vsPath << ", " << fsPath << std::endl;
    }
}
ShaderWrapper::~ShaderWrapper()
{
    if (m_shader.id > 0)
    {
        UnloadShader(m_shader);
    }
}
void ShaderWrapper::Begin() const { BeginShaderMode(m_shader); }
void ShaderWrapper::End() const { EndShaderMode(); }
int ShaderWrapper::GetLocation(const std::string &name)
{
    auto it = m_locationCache.find(name);
    if (it != m_locationCache.end())
        return it->second;

    int loc = GetShaderLocation(m_shader, name.c_str());
    m_locationCache[name] = loc;
    return loc;
}

void ShaderWrapper::SetAll(const Matrix4f &MVP, const Matrix4f &M, const Vector3f &viewPos, float realTime, float gameTime, const Vector4f &baseColor,

                           const std::unordered_map<std::string, float> &customFloats,
                           const std::unordered_map<std::string, Vector2f> &customVector2,
                           const std::unordered_map<std::string, Vector3f> &customVector3,
                           const std::unordered_map<std::string, Vector4f> &customVector4)
{
    SetMat4("u_mvp", MVP);
    SetMat4("transform", M);
    SetVec3("viewPos", viewPos);
    SetFloat("realTime", realTime);
    SetFloat("gameTime", gameTime);
    Vector4f color = baseColor / 255.0f;
    SetVec4("baseColor", color);

    for (auto const &[name, value] : customFloats)
        SetFloat(name, value);

    for (auto const &[name, value] : customVector2)
        SetVec2(name, value);
    for (auto const &[name, value] : customVector3)
        SetVec3(name, value);
    for (auto const &[name, value] : customVector4)
        SetVec4(name, value);
}

void ShaderWrapper::SetInt(const std::string &name, int value)
{
    int loc = GetLocation(name);
    if (loc >= 0)
        SetShaderValue(m_shader, loc, &value, SHADER_UNIFORM_INT);
}
void ShaderWrapper::SetFloat(const std::string &name, float value)
{
    int loc = GetLocation(name);
    if (loc >= 0)
        SetShaderValue(m_shader, loc, &value, SHADER_UNIFORM_FLOAT);
}

void ShaderWrapper::SetVec2(const std::string &name, const Vector2f &value)
{
    int loc = GetLocation(name);
    if (loc >= 0)
    {
        float valueArr[2] = {value.x(), value.y()};
        SetShaderValue(m_shader, loc, valueArr, SHADER_UNIFORM_VEC2);
    }
}
void ShaderWrapper::SetVec3(const std::string &name, const Vector3f &value)
{
    int loc = GetLocation(name);
    if (loc >= 0)
    {
        float valueArr[3] = {value.x(), value.y(), value.z()};
        SetShaderValue(m_shader, loc, valueArr, SHADER_UNIFORM_VEC3);
    }
}
void ShaderWrapper::SetVec4(const std::string &name, const Vector4f &value)
{
    int loc = GetLocation(name);
    if (loc >= 0)
    {
        float valueArr[4] = {value.x(), value.y(), value.z(), value.w()};
        SetShaderValue(m_shader, loc, valueArr, SHADER_UNIFORM_VEC4);
    }
}

void ShaderWrapper::SetMat4(const std::string &name, const Matrix4f &value)
{
    int loc = GetLocation(name);
    if (loc >= 0)
    {
        Matrix valueArr = value;
        SetShaderValueMatrix(m_shader, loc, valueArr);
    }
}
#include "rlgl.h"
void ShaderWrapper::SetTexture(const std::string &name, Texture2D texture, int unit)
{
    // TODO：贴图需重新实现
    int loc = GetLocation(name);
    if (loc >= 0)
    {
        SetShaderValue(m_shader, loc, &unit, SHADER_UNIFORM_INT);
        rlActiveTextureSlot(unit);
        rlEnableTexture(texture.id);
    }
}
