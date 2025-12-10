#pragma once
#include <string>
#include <vector>
#include <memory>
#include "vendor/glm/glm.hpp"

class ModelComponent;

// OBJ/MTL loader for 3D game models
// Loads .obj files and their associated .mtl material files
class OBJLoader {
public:
    struct Material {
        std::string name;
        glm::vec3 diffuseColor{1.0f, 1.0f, 1.0f};
        glm::vec3 specularColor{1.0f, 1.0f, 1.0f};
        glm::vec3 ambientColor{0.2f, 0.2f, 0.2f};
        float shininess = 32.0f;
        std::string diffuseTexture;
        float opacity = 1.0f;
    };

    // Load an OBJ file and convert it to a ModelComponent for 3D rendering
    static std::unique_ptr<ModelComponent> loadOBJ(const std::string& objPath);

private:
    static std::vector<Material> loadMTL(const std::string& mtlPath);
};
