#pragma once

#include "Camera.h"
#include "InputSystem.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <GLFW/glfw3.h>

class CameraController {
private:
    std::unique_ptr<Camera> activeCamera;
    std::unordered_map<std::string, std::unique_ptr<Camera>> cameras;
    std::string activeCameraName;
    
    InputSystem* inputSystem;
    
    // Mouse state
    float lastMouseX;
    float lastMouseY;
    bool firstMouse;
    bool mouseCaptured;

public:
    CameraController(InputSystem* input);
    
    void update(float deltaTime);
    
    // Camera management
    void addCamera(const std::string& name, std::unique_ptr<Camera> camera);
    void setActiveCamera(const std::string& name);
    Camera* getActiveCamera();
    const Camera* getActiveCamera() const;
    
    // Input
    void processInput(GLFWwindow* window, float deltaTime);
    void processMouseMovement(float xpos, float ypos);
    void processMouseScroll(float yoffset);
    
    // Mouse capture for camera control
    void captureMouse(bool capture);
    bool isMouseCaptured() const { return mouseCaptured; }
};
