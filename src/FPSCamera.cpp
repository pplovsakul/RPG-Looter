#include "FPSCamera.h"
#include <algorithm>

FPSCamera::FPSCamera(glm::vec3 pos)
    : Camera(pos), groundClipping(false), groundHeight(0.0f) {
    movementSpeed = 5.0f;
    mouseSensitivity = 0.1f;
}

void FPSCamera::processKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = movementSpeed * deltaTime;
    
    switch (direction) {
        case CameraMovement::Forward:
            position += front * velocity;
            break;
        case CameraMovement::Backward:
            position -= front * velocity;
            break;
        case CameraMovement::Left:
            position -= right * velocity;
            break;
        case CameraMovement::Right:
            position += right * velocity;
            break;
        case CameraMovement::Up:
            position += worldUp * velocity;
            break;
        case CameraMovement::Down:
            position -= worldUp * velocity;
            break;
    }
    
    // Apply ground clipping
    if (groundClipping) {
        position.y = std::max(position.y, groundHeight);
    }
    
    needsViewUpdate = true;
}

void FPSCamera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;
    
    yaw += xoffset;
    pitch += yoffset;
    
    // Constrain pitch to avoid screen flip
    if (constrainPitch) {
        pitch = std::clamp(pitch, -89.0f, 89.0f);
    }
    
    updateCameraVectors();
}

void FPSCamera::processMouseScroll(float yoffset) {
    zoom -= yoffset;
    zoom = std::clamp(zoom, 1.0f, 90.0f);
    needsProjectionUpdate = true;
}

void FPSCamera::setHeight(float height) {
    position.y = height;
    needsViewUpdate = true;
}

void FPSCamera::enableGroundClipping(bool enable, float height) {
    groundClipping = enable;
    groundHeight = height;
    
    if (groundClipping) {
        position.y = std::max(position.y, groundHeight);
        needsViewUpdate = true;
    }
}
