#include "CameraController.h"

CameraController::CameraController(InputSystem* input)
    : inputSystem(input), activeCameraName(""),
      lastMouseX(0.0f), lastMouseY(0.0f), firstMouse(true), mouseCaptured(false) {
}

void CameraController::update(float deltaTime) {
    // Update is handled in processInput
}

void CameraController::addCamera(const std::string& name, std::unique_ptr<Camera> camera) {
    cameras[name] = std::move(camera);
    
    // Set as active if it's the first camera
    if (cameras.size() == 1) {
        activeCameraName = name;
        activeCamera = nullptr; // Will be set on next getActiveCamera call
    }
}

void CameraController::setActiveCamera(const std::string& name) {
    auto it = cameras.find(name);
    if (it != cameras.end()) {
        activeCameraName = name;
        activeCamera = nullptr; // Force reload on next get
    }
}

Camera* CameraController::getActiveCamera() {
    if (!activeCamera && !activeCameraName.empty()) {
        auto it = cameras.find(activeCameraName);
        if (it != cameras.end()) {
            activeCamera = std::move(it->second);
        }
    }
    return activeCamera.get();
}

const Camera* CameraController::getActiveCamera() const {
    if (activeCamera) {
        return activeCamera.get();
    }
    
    auto it = cameras.find(activeCameraName);
    if (it != cameras.end()) {
        return it->second.get();
    }
    
    return nullptr;
}

void CameraController::processInput(GLFWwindow* window, float deltaTime) {
    Camera* camera = getActiveCamera();
    if (!camera) return;
    
    // Movement input
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->processKeyboard(CameraMovement::Forward, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->processKeyboard(CameraMovement::Backward, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->processKeyboard(CameraMovement::Left, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->processKeyboard(CameraMovement::Right, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera->processKeyboard(CameraMovement::Up, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera->processKeyboard(CameraMovement::Down, deltaTime);
}

void CameraController::processMouseMovement(float xpos, float ypos) {
    if (!mouseCaptured) {
        firstMouse = true;
        return;
    }
    
    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos; // Reversed since y-coordinates go from bottom to top
    
    lastMouseX = xpos;
    lastMouseY = ypos;
    
    Camera* camera = getActiveCamera();
    if (camera) {
        camera->processMouseMovement(xoffset, yoffset);
    }
}

void CameraController::processMouseScroll(float yoffset) {
    Camera* camera = getActiveCamera();
    if (camera) {
        camera->processMouseScroll(yoffset);
    }
}

void CameraController::captureMouse(bool capture) {
    mouseCaptured = capture;
    if (!capture) {
        firstMouse = true;
    }
}
