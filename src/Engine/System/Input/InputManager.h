#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "raylib.h"
struct ActionState {
    bool isDown = false;
    bool isPressed = false;
    bool isReleased = false;
};

struct AxisBinding {
    int positiveKey = -1; // "正方向" 按键 (例如, KEY_D)
    int negativeKey = -1; // "负方向" 按键 (例如, KEY_A)
    bool isMouse = false;  // 这个轴是否由鼠标移动驱动
    std::string mouseAxis; // "X" or "Y"
    float sensitivity = 1.0f;
};


class InputManager {
public:
    InputManager();
    ~InputManager() = default;

    bool LoadBindings(const std::string& filePath);

    // 在每帧的开始或结束时调用，用于更新所有动作的状态
    void Update();

    // 按住
    bool IsActionDown(const std::string& actionName) const;
    // 本帧按下
    bool IsActionPressed(const std::string& actionName) const;
    // 本帧松开
    bool IsActionReleased(const std::string& actionName) const;


    float GetAxisValue(const std::string& axisName) const;


    Vector2 GetMousePosition() const;
    Vector2 GetMouseDelta() const;
private:
    // 将字符串转换为 Raylib 的键盘/鼠标键码
    int KeyNameToKeyCode(const std::string& keyName) const;

    // 动作名称绑定的一个或多个键码的映射
    std::unordered_map<std::string, std::vector<int>> m_bindings;
    // 每个动作的当前帧状态
    std::unordered_map<std::string, ActionState> m_actionStates;

    std::unordered_map<std::string, AxisBinding> m_axisBindings;
    std::unordered_map<std::string, float> m_axisValues;
    Vector2 m_lastMousePosition; // 用于计算 mouseDelta
};