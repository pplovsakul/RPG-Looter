#pragma once
#include "Component.h"
#include "vendor/glm/glm.hpp"
#include <string>
#include "Texture.h"
#include <vector> // added

class TransformComponent : public Component {
public:
    glm::vec2 position = glm::vec2(0.0f);
    float rotation = 0.0f;
    glm::vec2 scale = glm::vec2(1.0f);
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

// --- ModelComponent: holds simple shape primitives to build a 2D model ---
class ModelComponent : public Component {
public:
    enum class ShapeType { Rectangle = 0, Triangle = 1, Circle = 2, TexturedQuad = 3 };

    struct Shape {
        ShapeType type = ShapeType::Rectangle;
        glm::vec2 position{0.0f, 0.0f}; // local position in model space (x right, y up)
        float rotation = 0.0f;          // degrees
        glm::vec2 size{50.0f, 50.0f};   // width/height or diameter for circle
        glm::vec2 scale{1.0f, 1.0f};    // additional scale multiplier
        glm::vec3 color{1.0f,1.0f,1.0f};
        std::string textureName;        // optional texture (works for all shapes; rendered as image inside bbox)
        bool filled = true;
        int layer = 0;                  // layer ordering (higher -> rendered later / on top)
    };

    std::vector<Shape> shapes;
};

