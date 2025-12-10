#pragma once
#include <GLFW/glfw3.h>
#include <unordered_map>

class InputSystem {
private:
    GLFWwindow* window;
    std::unordered_map<int, bool> keyState;

public:
    explicit InputSystem(GLFWwindow* win) : window(win) {}
    
    bool isKeyPressed(int key) const {
        return glfwGetKey(window, key) == GLFW_PRESS;
    }
    
    bool isKeyReleased(int key) const {
        return glfwGetKey(window, key) == GLFW_RELEASE;
    }
};
