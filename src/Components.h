#pragma once
#include "Component.h"
#include "vendor/glm/glm.hpp"
#include <string>
#include "Texture.h"
#include <vector> // added

class TransformComponent : public Component {
public:
    glm::vec3 position = glm::vec3(0.0f);      // 3D position in world space
    glm::vec3 rotation = glm::vec3(0.0f);      // Euler angles in radians: (pitch/X, yaw/Y, roll/Z)
    glm::vec3 scale = glm::vec3(1.0f);         // 3D scale factors
};

class RenderComponent : public Component {
public:
    // Enum for common mesh types for faster comparisons
    enum class MeshType { Unknown = 0, Quad = 1, Circle = 2, Triangle = 3 };
    
    std::string meshName = "quad";
    std::string shaderName = "default";
    std::string textureName = "";
    
    // Cached mesh type for performance (avoid string comparisons in hot path)
    MeshType meshType = MeshType::Quad;

    glm::vec3 color = glm::vec3(1.0f);
    float alpha = 1.0f;

    // Rendering options
    bool enabled = true;  // WICHTIG: enabled-Flag muss initialisiert sein!
    bool castsShadow = false;
    int renderLayer = 0;  // For sorting

    bool followMouse = false;
    
    // Helper to set mesh name and automatically update meshType
    void setMesh(const std::string& name) {
        meshName = name;
        if (name == "quad") meshType = MeshType::Quad;
        else if (name == "circle") meshType = MeshType::Circle;
        else if (name == "triangle") meshType = MeshType::Triangle;
        else meshType = MeshType::Unknown;
    }
};

class AudioComponent : public Component {
public:
    std::string soundName = "";  // Name des Sounds im AssetManager
    bool playOnce = false;       // Einmal abspielen und dann stoppen
    bool loop = false;           // Sound wiederholen
    float volume = 1.0f;         // 0.0 - 1.0
    float pitch = 1.0f;          // Tonhöhe
    bool shouldPlay = false;     // Trigger zum Abspielen
    bool isPlaying = false;      // Aktueller Status
};

// --- CameraComponent: 3D camera for perspective rendering ---
class CameraComponent : public Component {
public:
    float fov = 60.0f;                         // Field of view in degrees
    float nearPlane = 0.1f;                    // Near clipping plane
    float farPlane = 1000.0f;                  // Far clipping plane
    bool isActive = true;                      // Whether this is the active camera
    
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);  // Camera forward direction
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);      // Camera up direction
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);   // Camera right direction
    
    // Calculate view matrix from transform
    glm::mat4 getViewMatrix(const TransformComponent* transform) const {
        if (!transform) return glm::mat4(1.0f);
        return glm::lookAt(transform->position, 
                          transform->position + front, 
                          up);
    }
    
    // Update camera direction vectors from rotation
    void updateVectors(const glm::vec3& rotation) {
        glm::vec3 newFront;
        newFront.x = cos(rotation.y) * cos(rotation.x);
        newFront.y = sin(rotation.x);
        newFront.z = sin(rotation.y) * cos(rotation.x);
        front = glm::normalize(newFront);
        right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        up = glm::normalize(glm::cross(right, front));
    }
};


class UIComponent : public Component {
public:
    enum class Type {
        Text,
        Button
    };

    Type type = Type::Text;
    std::string text;
    glm::vec2 position{ 0.0f, 0.0f };
    glm::vec2 size{ 0.0f, 0.0f };
    glm::vec3 color{ 1.0f, 1.0f, 1.0f };
    bool visible = true;

    // Nur für Buttons
    std::function<void()> onClick = nullptr;
    bool isHovered = false;
};

// --- ModelComponent: holds 3D mesh data from OBJ files ---
class ModelComponent : public Component {
public:
    struct Mesh {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> indices;
        std::string materialName;
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        std::string textureName;
        
        // Cached OpenGL buffers (initialized on first render)
        mutable unsigned int VAO = 0;
        mutable unsigned int VBO = 0;
        mutable unsigned int EBO = 0;
        mutable bool buffersInitialized = false;
    };

    std::vector<Mesh> meshes;
    
    ~ModelComponent() {
        // Clean up OpenGL buffers
        for (auto& mesh : meshes) {
            if (mesh.buffersInitialized) {
                glDeleteVertexArrays(1, &mesh.VAO);
                glDeleteBuffers(1, &mesh.VBO);
                glDeleteBuffers(1, &mesh.EBO);
            }
        }
    }
};

// --- MeshComponent: holds reference to 3D mesh ---
class MeshComponent : public Component {
public:
    std::string meshName;  // Reference to mesh in AssetManager
    
    // Material properties
    std::string materialName;
    glm::vec3 diffuseColor{1.0f, 1.0f, 1.0f};
    glm::vec3 specularColor{0.5f, 0.5f, 0.5f};
    float shininess = 32.0f;
    
    // Rendering flags
    bool castShadows = true;
    bool receiveShadows = true;
    bool visible = true;
    int renderLayer = 0;
    
    // Debug
    bool wireframe = false;
};

// --- CharacterControllerComponent: physics-based movement ---
class CharacterControllerComponent : public Component {
public:
    // Movement
    float moveSpeed = 5.0f;
    float sprintMultiplier = 2.0f;
    float jumpForce = 5.0f;
    
    // Physics
    glm::vec3 velocity{0.0f};
    bool isGrounded = false;
    bool isJumping = false;
    
    // Input state
    glm::vec2 moveInput{0.0f}; // x: left/right, y: forward/backward
    bool wantsToJump = false;
    bool isSprinting = false;
    
    // Configuration
    bool useGravity = true;
    float gravity = -9.81f;
    float groundCheckDistance = 0.1f;
    
    // Collision
    float radius = 0.5f; // Capsule radius
    float height = 2.0f; // Capsule height
};

