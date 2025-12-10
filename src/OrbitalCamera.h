#pragma once

#include "Camera.h"

class OrbitalCamera : public Camera {
private:
    glm::vec3 target;
    float distance;
    float minDistance;
    float maxDistance;
    
    float azimuth;   // Horizontal rotation
    float elevation; // Vertical rotation
    
    float panSpeed;

public:
    OrbitalCamera(glm::vec3 targetPos = glm::vec3(0.0f), float dist = 5.0f);
    
    void processKeyboard(CameraMovement direction, float deltaTime) override;
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) override;
    void processMouseScroll(float yoffset) override;
    
    // Orbital-specific functions
    void setTarget(const glm::vec3& newTarget);
    void setDistance(float dist);
    void setDistanceLimits(float min, float max);
    
    void rotate(float deltaAzimuth, float deltaElevation);
    void pan(float deltaX, float deltaY);
    void zoom(float delta);
    
    glm::vec3 getTarget() const { return target; }
    float getDistance() const { return distance; }

private:
    void updatePositionFromAngles();
};
