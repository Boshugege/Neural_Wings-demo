#pragma once
#include <string>
#include "raylib.h"
#include "Engine/Math/Math.h"
#include <unordered_map>

class ShaderWrapper
{
public:
    ShaderWrapper(const std::string &vsPath, const std::string &fsPath);

    // 用于TFB
    // varyings: 从 Vertex Shader 捕获并写回 Buffer 的变量（如 "out_position", "out_velocity"）
    ShaderWrapper(const std::string &vsPath, const std::vector<std::string> &varyings);
    ~ShaderWrapper();

    ShaderWrapper(const ShaderWrapper &) = delete;
    ShaderWrapper &operator=(const ShaderWrapper &) = delete;

    void Begin() const;
    void End() const;

    // Uniform 接口
    void SetInt(const std::string &name, int value);
    void SetFloat(const std::string &name, float value);

    void SetVec2(const std::string &name, const Vector2f &value);
    void SetVec3(const std::string &name, const Vector3f &value);
    void SetVec4(const std::string &name, const Vector4f &value);
    void SetMat4(const std::string &name, const Matrix4f &value);
    void SetTexture(const std::string &name, Texture2D texture, int unit);

    bool IsValid() const { return m_shader.id > 0; };
    Shader GetShader() const { return m_shader; }

    void SetAll(const Matrix4f &MVP, const Matrix4f &M, const Vector3f &viewPos, float realTime, float gameTime, const Vector4f &baseColor,
                const std::unordered_map<std::string, float> &customFloats,
                const std::unordered_map<std::string, Vector2f> &customVector2,
                const std::unordered_map<std::string, Vector3f> &customVector3,
                const std::unordered_map<std::string, Vector4f> &customVector4);

private:
    int GetLocation(const std::string &name);

    std::string LoadVSText(const std::string &path);
    Shader m_shader;
    std::unordered_map<std::string, int> m_locationCache;
};