#pragma once

#include "Camera.h"

class FPSCamera : public Camera {
private:
    bool groundClipping;
    float groundHeight;

public:
    FPSCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f));
    
    void processKeyboard(CameraMovement direction, float deltaTime) override;
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) override;
    void processMouseScroll(float yoffset) override;
    
    // FPS-specific functions
    void setHeight(float height);
    void enableGroundClipping(bool enable, float height = 0.0f);
};
