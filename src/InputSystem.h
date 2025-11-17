#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include "vendor/glm/glm.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "System.h"
#include "JsonParser.h"

enum class InputState {
    Pressed,
    Released,
    Held
};

struct InputAction {
    std::string name;
    std::function<void(InputState, EntityManager&, float)> callback;
};

struct InputBinding {
    int key = -1;           
    int gamepadButton = -1; 
    bool isAxis = false;    
};

struct InputContext {
    std::string name;
    std::unordered_map<std::string, std::vector<InputBinding>> bindings;
};

class InputSystem : public System {
private:
    GLFWwindow* window = nullptr;
    std::unordered_map<std::string, InputContext> contexts;
    std::vector<std::string> activeContexts;
    std::unordered_map<std::string, InputAction> actions;
    std::unordered_map<int, bool> keyState;
    
    // Mouse state tracking
    glm::vec2 mousePosition{0.0f, 0.0f};
    std::unordered_map<int, bool> mouseButtonState;
    bool mouseOverUI = false;

public:
    // Constructor
    explicit InputSystem(GLFWwindow* win);
    InputSystem() = default;

    // === Initialization ===
    void init(GLFWwindow* win);

    // === Context Management ===
    void addContext(const InputContext& ctx);
    void activateContext(const std::string& name);
    void deactivateContext(const std::string& name);
    bool isContextActive(const std::string& name) const;

    // === Action Registration ===
    void registerAction(const std::string& name, std::function<void(InputState, EntityManager&, float)> callback);

    // === Main Update ===
    // Implement System interface
    void update(EntityManager& em, float deltaTime) override;

    // === Utility ===
    bool isKeyDown(int key) const;
    bool isKeyPressedOnce(int key);

    void loadConfig(const std::string& path);
    
    // === Mouse Support ===
    glm::vec2 GetMousePosition() const;
    bool IsMouseButtonPressed(int button) const;
    
    // === UI Input Consumption ===
    void SetMouseOverUI(bool overUI);
    bool IsMouseOverUI() const;
};