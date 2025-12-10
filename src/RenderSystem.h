#pragma once
#include "System.h"
#include "AssetManager.h"
#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"

class RenderSystem : public System {
private:
    AssetManager* assetManager;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    unsigned int fallbackShaderID = 0;
    bool warnedNoRender = false;
    
    // Cache for sorted entities to avoid sorting every frame
    std::vector<Entity*> sortedEntities;
    bool needsResort = true;
    
    // Cache current program to avoid repeated glGetIntegerv calls
    mutable GLuint cachedProgramID = 0;
    
    // Lighting
    glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 5.0f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);

public:

    RenderSystem();
    ~RenderSystem() = default;

    void init();
    void update(EntityManager& em, float deltaTime) override;

    // Camera
    void setViewMatrix(const glm::mat4& view);
    void setProjectionMatrix(const glm::mat4& proj);
    
    // Lighting
    void setLightPosition(const glm::vec3& pos) { lightPos = pos; }
    void setLightColor(const glm::vec3& color) { lightColor = color; }
    void setAmbientColor(const glm::vec3& color) { ambientColor = color; }
    
    // Mark that entities need resorting (call when entities added/removed or layers change)
    void markNeedsResort() { needsResort = true; }

private:
    void createDefaultAssets();
    void createFallbackShader();
    void renderMesh(const ModelComponent::Mesh& mesh, const glm::mat4& modelMatrix, 
                   const glm::vec3& cameraPos);
};