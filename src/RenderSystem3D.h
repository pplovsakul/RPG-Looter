#pragma once

#include "System.h"
#include "AssetManager.h"
#include "CameraController.h"
#include "Mesh3D.h"
#include "Components.h"
#include "vendor/glm/glm.hpp"
#include <memory>
#include <unordered_map>

class RenderSystem3D : public System {
private:
    AssetManager* assetManager;
    CameraController* cameraController;
    
    // Shaders
    unsigned int defaultShaderID;
    unsigned int lightingShaderID;
    
    // Lighting
    glm::vec3 lightDirection;
    glm::vec3 lightColor;
    glm::vec3 ambientColor;
    
    // Mesh storage
    std::unordered_map<std::string, std::shared_ptr<Mesh3D>> meshes;

public:
    RenderSystem3D(AssetManager* assets, CameraController* camera);
    
    void init();
    void update(EntityManager& em, float deltaTime) override;
    
    // Mesh management
    void addMesh(const std::string& name, std::shared_ptr<Mesh3D> mesh);
    std::shared_ptr<Mesh3D> getMesh(const std::string& name);
    
    // Lighting
    void setDirectionalLight(const glm::vec3& direction, const glm::vec3& color);
    void setAmbientLight(const glm::vec3& color);
    
    glm::vec3 getLightDirection() const { return lightDirection; }
    glm::vec3 getLightColor() const { return lightColor; }
    glm::vec3 getAmbientColor() const { return ambientColor; }

private:
    void createDefaultShaders();
    void renderMesh(const MeshComponent* meshComp, const TransformComponent* transform);
    void setupLighting(unsigned int shaderID);
};
