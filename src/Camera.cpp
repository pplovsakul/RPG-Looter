#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 worldUpVec, float yawAngle, float pitchAngle)
    : position(pos), worldUp(worldUpVec), yaw(yawAngle), pitch(pitchAngle),
      front(glm::vec3(0.0f, 0.0f, -1.0f)),
      movementSpeed(2.5f), mouseSensitivity(0.1f), zoom(45.0f),
      aspectRatio(16.0f / 9.0f), nearPlane(0.1f), farPlane(100.0f),
      needsViewUpdate(true), needsProjectionUpdate(true) {
    
    updateCameraVectors();
}

const glm::mat4& Camera::getViewMatrix() {
    if (needsViewUpdate) {
        updateViewMatrix();
        needsViewUpdate = false;
    }
    return viewMatrix;
}

const glm::mat4& Camera::getProjectionMatrix() {
    if (needsProjectionUpdate) {
        updateProjectionMatrix();
        needsProjectionUpdate = false;
    }
    return projectionMatrix;
}

void Camera::setAspectRatio(float aspect) {
    aspectRatio = aspect;
    needsProjectionUpdate = true;
}

void Camera::setClippingPlanes(float near, float far) {
    nearPlane = near;
    farPlane = far;
    needsProjectionUpdate = true;
}

void Camera::updateCameraVectors() {
    // Calculate the new Front vector
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    
    // Re-calculate the Right and Up vector
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
    
    needsViewUpdate = true;
}

void Camera::updateViewMatrix() {
    viewMatrix = glm::lookAt(position, position + front, up);
}

void Camera::updateProjectionMatrix() {
    projectionMatrix = glm::perspective(glm::radians(zoom), aspectRatio, nearPlane, farPlane);
}
