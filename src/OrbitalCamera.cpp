#include "OrbitalCamera.h"
#include <algorithm>

OrbitalCamera::OrbitalCamera(glm::vec3 targetPos, float dist)
    : Camera(glm::vec3(0.0f, 0.0f, dist)),
      target(targetPos), distance(dist), minDistance(1.0f), maxDistance(50.0f),
      azimuth(0.0f), elevation(20.0f), panSpeed(0.01f) {
    
    movementSpeed = 0.05f;
    mouseSensitivity = 0.2f;
    updatePositionFromAngles();
}

void OrbitalCamera::processKeyboard(CameraMovement direction, float deltaTime) {
    float panVelocity = movementSpeed * deltaTime * distance;
    
    switch (direction) {
        case CameraMovement::Forward:
            zoom(-1.0f);
            break;
        case CameraMovement::Backward:
            zoom(1.0f);
            break;
        case CameraMovement::Left:
            pan(-panVelocity, 0.0f);
            break;
        case CameraMovement::Right:
            pan(panVelocity, 0.0f);
            break;
        case CameraMovement::Up:
            pan(0.0f, panVelocity);
            break;
        case CameraMovement::Down:
            pan(0.0f, -panVelocity);
            break;
    }
}

void OrbitalCamera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    rotate(xoffset * mouseSensitivity, -yoffset * mouseSensitivity);
}

void OrbitalCamera::processMouseScroll(float yoffset) {
    zoom(-yoffset);
}

void OrbitalCamera::setTarget(const glm::vec3& newTarget) {
    target = newTarget;
    updatePositionFromAngles();
}

void OrbitalCamera::setDistance(float dist) {
    distance = std::clamp(dist, minDistance, maxDistance);
    updatePositionFromAngles();
}

void OrbitalCamera::setDistanceLimits(float min, float max) {
    minDistance = min;
    maxDistance = max;
    distance = std::clamp(distance, minDistance, maxDistance);
}

void OrbitalCamera::rotate(float deltaAzimuth, float deltaElevation) {
    azimuth += deltaAzimuth;
    elevation += deltaElevation;
    
    // Constrain elevation
    elevation = std::clamp(elevation, -89.0f, 89.0f);
    
    // Wrap azimuth
    while (azimuth > 360.0f) azimuth -= 360.0f;
    while (azimuth < 0.0f) azimuth += 360.0f;
    
    updatePositionFromAngles();
}

void OrbitalCamera::pan(float deltaX, float deltaY) {
    target += right * deltaX + up * deltaY;
    updatePositionFromAngles();
}

void OrbitalCamera::zoom(float delta) {
    distance -= delta * 0.5f;
    distance = std::clamp(distance, minDistance, maxDistance);
    updatePositionFromAngles();
}

void OrbitalCamera::updatePositionFromAngles() {
    // Calculate position based on spherical coordinates
    float azimuthRad = glm::radians(azimuth);
    float elevationRad = glm::radians(elevation);
    
    position.x = target.x + distance * cos(elevationRad) * sin(azimuthRad);
    position.y = target.y + distance * sin(elevationRad);
    position.z = target.z + distance * cos(elevationRad) * cos(azimuthRad);
    
    // Update front vector to look at target
    front = glm::normalize(target - position);
    
    // Update right and up vectors
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
    
    needsViewUpdate = true;
}
