#pragma once
#include "System.h"
#include "AssetManager.h"
#include "vendor/glm/glm.hpp"

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

public:

    RenderSystem();
    ~RenderSystem() = default;

    void init();
    void update(EntityManager& em, float deltaTime) override;

    // Camera
    void setViewMatrix(const glm::mat4& view);
    void setProjectionMatrix(const glm::mat4& proj);
    
    // Mark that entities need resorting (call when entities added/removed or layers change)
    void markNeedsResort() { needsResort = true; }

private:
    void createDefaultAssets();
    void createFallbackShader();
};