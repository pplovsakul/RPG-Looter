#pragma once

#include <memory>
#include "vendor/glm/glm.hpp"

// Forward declarations
class Mesh;
class Shader;

// InputState structure for handling player input
// This provides a simple interface for input handling
struct InputState {
    bool up = false;       // W key or forward movement
    bool down = false;     // S key or backward movement
    bool left = false;     // A key or left movement
    bool right = false;    // D key or right movement
    bool jump = false;     // Space key
    glm::vec2 mouseDelta{0.0f, 0.0f}; // Mouse movement
};

/**
 * Player class - Manages player position, movement, and rendering
 * 
 * This class stores a mesh per player and handles:
 * - Position and velocity management
 * - Input handling (WASD movement)
 * - Rendering with model matrix transformation
 * - Optional gravity simulation
 * 
 * Note: This class does NOT handle health/damage/healing mechanics
 */
class Player {
public:
    // Constructors
    Player();
    explicit Player(const glm::vec3& startPos);

    // Core update methods
    void Update(float dt); // dt in seconds
    void HandleInput(const InputState& input);
    void Draw(Shader& shader) const; // Shader must be bound before calling

    // Mesh management
    void SetMesh(std::shared_ptr<Mesh> mesh);
    std::shared_ptr<Mesh> GetMesh() const;

    // Position management
    const glm::vec3& GetPosition() const;
    void SetPosition(const glm::vec3& position);
    void Teleport(const glm::vec3& position); // Instantly set position (alias for SetPosition)

    // Speed management
    void SetSpeed(float speed);
    float GetSpeed() const;

    // Gravity toggle
    void SetUseGravity(bool enabled);
    bool GetUseGravity() const;

private:
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    float m_speed = 5.0f;
    std::shared_ptr<Mesh> m_mesh;
    bool m_useGravity = false;
};
