#pragma once

#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"

enum class CameraMovement {
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

class Camera {
protected:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    
    // Euler Angles
    float yaw;
    float pitch;
    
    // Camera options
    float movementSpeed;
    float mouseSensitivity;
    float zoom; // FOV
    
    // Projection
    float aspectRatio;
    float nearPlane;
    float farPlane;
    
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    
    bool needsViewUpdate;
    bool needsProjectionUpdate;

public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = 0.0f);
    
    virtual ~Camera() = default;
    
    // Interface for different camera modes
    virtual void processKeyboard(CameraMovement direction, float deltaTime) = 0;
    virtual void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) = 0;
    virtual void processMouseScroll(float yoffset) = 0;
    
    // Matrix Getters
    const glm::mat4& getViewMatrix();
    const glm::mat4& getProjectionMatrix();
    
    // Setters
    void setAspectRatio(float aspect);
    void setClippingPlanes(float near, float far);
    
    // Getters
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    glm::vec3 getUp() const { return up; }
    glm::vec3 getRight() const { return right; }
    float getZoom() const { return zoom; }
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }

protected:
    void updateCameraVectors();
    void updateViewMatrix();
    void updateProjectionMatrix();
};
