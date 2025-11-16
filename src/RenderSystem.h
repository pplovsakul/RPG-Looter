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

public:

    RenderSystem();
    ~RenderSystem() = default;

    void init();
    void update(EntityManager& em, float deltaTime) override;

    // Camera
    void setViewMatrix(const glm::mat4& view);
    void setProjectionMatrix(const glm::mat4& proj);

private:
    void createDefaultAssets();
    void createFallbackShader();
};