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

// TFB
#ifdef GL_INTERLEAVED_ATTRIBS
#define GL_INTERLEAVED_ATTRIBS 0x8C8C
#endif
#if defined(PLATFORM_WEB)
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#else
#include "external/glad.h"
#endif

#include "rlgl.h"
#include <vector>
ShaderWrapper::ShaderWrapper(const std::string &vsPath, const std::vector<std::string> &varyings)
{
    m_shader.id = 0;
    std::string vsCode = LoadVSText(vsPath);
    if (vsCode.empty())
    {
        std::cerr << "[ShaderWrapper] TFB VS Source empty: " << vsPath << std::endl;
        return;
    }
    unsigned int vShaderId = rlCompileShader(vsCode.c_str(), RL_VERTEX_SHADER);
    if (vShaderId == 0)
        return;

    // raylib源码修改：插入varyings
    // rlLoadShaderProgram();
    unsigned int programId = glCreateProgram();
    glAttachShader(programId, vShaderId);

    // 编写shader时规范，顶点着色器包含这些变量
    glBindAttribLocation(programId, 0, "pPosition");     // vec3
    glBindAttribLocation(programId, 1, "pVelocity");     // vec3
    glBindAttribLocation(programId, 2, "pAcceleration"); // vec3

    glBindAttribLocation(programId, 3, "pColor");    // vec4
    glBindAttribLocation(programId, 4, "pSize");     // vec2
    glBindAttribLocation(programId, 5, "pRotation"); // float
    glBindAttribLocation(programId, 6, "pLife");     // vec2: (totalLife,remainingLife )
    glBindAttribLocation(programId, 7, "pRandomID"); // 随机数种子

    // glLinkProgram前注入varyings
    std::vector<const char *> varyingsRaw;
    for (const auto &s : varyings)
        varyingsRaw.push_back(s.c_str());
    glTransformFeedbackVaryings(programId, (int)varyings.size(), varyingsRaw.data(), GL_INTERLEAVED_ATTRIBS);

    glLinkProgram(programId);

    int success = 0;
    glGetProgramiv(programId, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        int maxLength = 0;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            char *infoLog = new char[maxLength];
            glGetProgramInfoLog(programId, maxLength, NULL, infoLog);
            std::cerr << "[ShaderWrapper] TFB link shader: " << vsPath << ", " << infoLog << std::endl;
            delete[] infoLog;
        }

        glDeleteProgram(programId);
        programId = 0;
    }
    else
        std::cout << "[ShaderWrapper] TFB Program ID: " << programId << " linked successfully" << std::endl;
    glDeleteShader(vShaderId);
    m_shader.id = programId;
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
// TODO:使用模板实现自定义参数上传
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
    int loc = GetLocation(name);
    if (loc >= 0)
    {
        SetShaderValue(m_shader, loc, &unit, SHADER_UNIFORM_INT);
        rlActiveTextureSlot(unit);
        rlEnableTexture(texture.id);
    }
}

// private:
int ShaderWrapper::GetLocation(const std::string &name)
{
    auto it = m_locationCache.find(name);
    if (it != m_locationCache.end())
        return it->second;

    int loc = GetShaderLocation(m_shader, name.c_str());
    m_locationCache[name] = loc;
    return loc;
}

#include <fstream>
#include <sstream>
std::string ShaderWrapper::LoadVSText(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[ShaderWrapper] Failed to open file: " << path << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}