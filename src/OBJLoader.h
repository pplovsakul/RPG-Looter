#pragma once
#include <string>
#include <vector>
#include <memory>
#include "vendor/glm/glm.hpp"

class ModelComponent;

// Simple OBJ/MTL loader for 2D game models
// Loads .obj files and their associated .mtl material files
class OBJLoader {
public:
    struct Material {
        std::string name;
        glm::vec3 diffuseColor{1.0f, 1.0f, 1.0f};
        std::string diffuseTexture;
        float opacity = 1.0f;
    };

    struct Mesh {
        std::string name;
        std::string materialName;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uvs;
        std::vector<unsigned int> indices;
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        std::string textureName;
    };

    // Load an OBJ file and convert it to a ModelComponent for 2D rendering
    static std::unique_ptr<ModelComponent> loadOBJ(const std::string& objPath);

private:
    static std::vector<Material> loadMTL(const std::string& mtlPath);
    static std::unique_ptr<ModelComponent> convertToModelComponent(
        const std::vector<Mesh>& meshes,
        const std::vector<Material>& materials
    );
};
