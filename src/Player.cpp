#include "Player.h"
#include "Mesh.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// Default constructor - initializes player at origin
Player::Player()
    : m_position(0.0f, 0.0f, 0.0f)
    , m_velocity(0.0f, 0.0f, 0.0f)
    , m_speed(5.0f)
    , m_mesh(nullptr)
    , m_useGravity(false)
{
}

// Constructor with starting position
Player::Player(const glm::vec3& startPos)
    : m_position(startPos)
    , m_velocity(0.0f, 0.0f, 0.0f)
    , m_speed(5.0f)
    , m_mesh(nullptr)
    , m_useGravity(false)
{
}

// Update player state based on delta time
void Player::Update(float dt)
{
    // Apply gravity if enabled
    if (m_useGravity) {
        m_velocity.y -= 9.81f * dt; // Standard gravity acceleration
    }

    // Update position based on velocity
    m_position += m_velocity * dt;
}

// Handle input and update velocity
void Player::HandleInput(const InputState& input)
{
    glm::vec3 direction(0.0f, 0.0f, 0.0f);

    // Calculate movement direction based on input
    // Note: Using standard 3D coordinate system where:
    // - X axis: left (-) / right (+)
    // - Y axis: down (-) / up (+)
    // - Z axis: forward (+) / backward (-)
    
    if (input.up) {
        direction.z += 1.0f;  // Forward
    }
    if (input.down) {
        direction.z -= 1.0f;  // Backward
    }
    if (input.left) {
        direction.x -= 1.0f;  // Left
    }
    if (input.right) {
        direction.x += 1.0f;  // Right
    }

    // Normalize direction to prevent faster diagonal movement
    if (glm::length(direction) > 0.0f) {
        direction = glm::normalize(direction);
    }

    // Set velocity based on direction and speed
    // Note: For horizontal movement only, we don't override Y velocity
    // This allows gravity to continue working while moving
    m_velocity.x = direction.x * m_speed;
    m_velocity.z = direction.z * m_speed;
    
    // If gravity is disabled, also handle Y velocity from direction
    if (!m_useGravity) {
        m_velocity.y = direction.y * m_speed;
    }
}

// Draw the player
void Player::Draw(const Shader& shader) const
{
    // Check if mesh is set
    if (!m_mesh) {
        // No mesh to draw - silently return
        // Note: This is not an error condition, just means the player has no visual representation yet
        return;
    }

    // Ensure mesh has GL objects set up
    // This check is important because SetupGL requires a valid OpenGL context
    if (!m_mesh->HasGL()) {
        // Try to setup GL objects
        // Note: This assumes the GL context is current and GLAD is initialized
        const_cast<Mesh*>(m_mesh.get())->SetupGL();
        
        if (!m_mesh->HasGL()) {
            std::cerr << "Player::Draw: Failed to setup GL objects for mesh. "
                      << "Ensure OpenGL context is current and GLAD is initialized." << std::endl;
            return;
        }
    }

    // Create model matrix for this player's position
    // Start with identity matrix and translate to player position
    glm::mat4 model = glm::translate(glm::mat4(1.0f), m_position);
    
    // Set the model matrix uniform in the shader
    // Note: The shader must be bound before calling Draw()
    // The uniform name "u_Model" is assumed - adjust if your shader uses a different name
    const_cast<Shader&>(shader).SetUniformMat4f("u_Model", model);
    
    // Draw the mesh
    m_mesh->Draw();
}

// Mesh management
void Player::SetMesh(std::shared_ptr<Mesh> mesh)
{
    m_mesh = mesh;
}

std::shared_ptr<Mesh> Player::GetMesh() const
{
    return m_mesh;
}

// Position management
const glm::vec3& Player::GetPosition() const
{
    return m_position;
}

void Player::SetPosition(const glm::vec3& position)
{
    m_position = position;
}

void Player::Teleport(const glm::vec3& position)
{
    // Teleport is just an alias for SetPosition - instantly moves player
    SetPosition(position);
}

// Speed management
void Player::SetSpeed(float speed)
{
    m_speed = speed;
}

float Player::GetSpeed() const
{
    return m_speed;
}

// Gravity management
void Player::SetUseGravity(bool enabled)
{
    m_useGravity = enabled;
}

bool Player::GetUseGravity() const
{
    return m_useGravity;
}
