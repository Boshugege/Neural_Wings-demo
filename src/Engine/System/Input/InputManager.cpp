#include "InputManager.h"
#include "raylib.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

InputManager::InputManager() {
    LoadBindings("assets/config/default/input_config.json");
    m_lastMousePosition=GetMousePosition();
}

bool InputManager::LoadBindings(const std::string& filePath) {
    m_bindings.clear();

    std::ifstream configFile(filePath);
    if (!configFile.is_open()) {
        std::cerr << "Error: [InputManager] Could not open file: " << filePath << std::endl;
        return false;
    }

    json bindingsJson;
    try {
        configFile >> bindingsJson;
        for (const auto& binding : bindingsJson["keybindings"]) {
            std::string action = binding["action"];
            for (const auto& key : binding["keys"]) {
                int keyCode = KeyNameToKeyCode(key.get<std::string>());
                if (keyCode != -1) { 
                    m_bindings[action].push_back(keyCode);
                }
            }
        }
    } catch (json::parse_error& e) {
        std::cerr << "Error: [InputManager] Failed to parse " << filePath << ": " << e.what() << std::endl;
        return false;
    }

    m_axisBindings.clear();
    try {
        if (bindingsJson.contains("axisbindings")) {
            for (const auto& binding : bindingsJson["axisbindings"]) {
                std::string axisName = binding["axis"];
                AxisBinding axisBinding;
                axisBinding.sensitivity = binding.value("sensitivity", 1.0f);

                std::string type = binding.value("type", "Keys");
                if (type == "MouseX" || type == "MouseY") {
                    axisBinding.isMouse = true;
                    axisBinding.mouseAxis = (type == "MouseX") ? "X" : "Y";
                    if (binding.value("invert", false)) {
                        axisBinding.sensitivity *= -1.0f; // 反转灵敏度
                    }
                } else { // type == "Keys"
                    if (binding.contains("positiveKey")) {
                        axisBinding.positiveKey = KeyNameToKeyCode(binding["positiveKey"]);
                    }
                    if (binding.contains("negativeKey")) {
                        axisBinding.negativeKey = KeyNameToKeyCode(binding["negativeKey"]);
                    }
                }
                m_axisBindings[axisName] = axisBinding;
                m_axisValues[axisName] = 0.0f; // 初始化轴值为0
            }
        }
    } catch(json::parse_error& e) {
        std::cerr << "Error: [InputManager] Failed to parse " << filePath << ": " << e.what() << std::endl;
        return false;
    }



    configFile.close();
    for (const auto& pair : m_bindings) {
        m_actionStates[pair.first] = ActionState();
    }

    return true;
}

void InputManager::Update() {
    for (auto& pair : m_actionStates) {
        const std::string& actionName = pair.first;
        ActionState& state = pair.second;

        bool isCurrentlyDown = false;
        // 检查这个动作绑定的所有按键
        const auto& keyCodes = m_bindings[actionName];
        for (int keyCode : keyCodes) {
            // TODO:raylib检测按键是否按下，移动端重写此处
            if (IsKeyDown(keyCode) || IsMouseButtonDown(keyCode)) {
                isCurrentlyDown = true;
                break;
            }
        }

        // 更新状态
        state.isPressed = !state.isDown && isCurrentlyDown;
        state.isReleased = state.isDown && !isCurrentlyDown;
        // isDown: 更新为当前状态
        state.isDown = isCurrentlyDown;

    }

    // 更新轴值
    Vector2 mouseDelta = GetMouseDelta();

    for (auto& pair : m_axisValues) {
        const std::string& axisName = pair.first;
        float& axisValue = pair.second;
        const AxisBinding& binding = m_axisBindings[axisName];

        axisValue = 0.0f; // 每帧重置

        if (binding.isMouse) {
            if (binding.mouseAxis == "X") {
                axisValue = mouseDelta.x * binding.sensitivity;
            } else { // "Y"
                axisValue = mouseDelta.y * binding.sensitivity;
            }
        } else { // Keyboard keys
            if (binding.positiveKey != -1 && IsKeyDown(binding.positiveKey)) {
                axisValue += 1.0f;
            }
            if (binding.negativeKey != -1 && IsKeyDown(binding.negativeKey)) {
                axisValue -= 1.0f;
            }
        }
    }

    // 更新上一帧的鼠标位置
    m_lastMousePosition = GetMousePosition();
}
float InputManager::GetAxisValue(const std::string& axisName) const {
    auto it = m_axisValues.find(axisName);
    return (it != m_axisValues.end()) ? it->second : 0.0f;
}

Vector2 InputManager::GetMousePosition() const {
    return ::GetMousePosition(); // 调用 Raylib 的全局函数
}

Vector2 InputManager::GetMouseDelta() const {
    return ::GetMouseDelta(); // Raylib 5.0 提供函数
  }


bool InputManager::IsActionDown(const std::string& actionName) const {
    auto it = m_actionStates.find(actionName);
    return (it != m_actionStates.end()) ? it->second.isDown : false;
}

bool InputManager::IsActionPressed(const std::string& actionName) const {
    auto it = m_actionStates.find(actionName);
    return (it != m_actionStates.end()) ? it->second.isPressed : false;
}

bool InputManager::IsActionReleased(const std::string& actionName) const {
    auto it = m_actionStates.find(actionName);
    return (it != m_actionStates.end()) ? it->second.isReleased : false;
}

// TODO:转换到raylib的按键代码
int InputManager::KeyNameToKeyCode(const std::string& keyName) const {
    // 字母
    if (keyName.length() == 1 && keyName[0] >= 'A' && keyName[0] <= 'Z') {
        return static_cast<int>(keyName[0]);
    }
    // 方向键
    if (keyName == "UP") return KEY_UP;
    if (keyName == "DOWN") return KEY_DOWN;
    if (keyName == "LEFT") return KEY_LEFT;
    if (keyName == "RIGHT") return KEY_RIGHT;
    // 功能键
    if (keyName == "LEF_SHIFT") return KEY_LEFT_SHIFT;
    if (keyName == "SPACE") return KEY_SPACE;
    if (keyName == "ENTER") return KEY_ENTER;
    if (keyName == "LEFT_CONTROL") return KEY_LEFT_CONTROL;

    if (keyName == "ESC") return KEY_ESCAPE;

    // 鼠标
    if (keyName == "MOUSE_LEFT_BUTTON") return MOUSE_BUTTON_LEFT;
    if (keyName == "MOUSE_RIGHT_BUTTON") return MOUSE_BUTTON_RIGHT;
    
    std::cerr << "Warning: [InputManager] Unknown key name: " << keyName << std::endl;
    return -1;
}