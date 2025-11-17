#pragma once
#include "UIWidget.h"
#include "../Shader.h"
#include "../VertexArray.h"
#include "../VertexBuffer.h"
#include "../IndexBuffer.h"
#include "../vendor/glm/glm.hpp"
#include <memory>
#include <vector>

class UIRenderer {
private:
    std::unique_ptr<Shader> uiShader;
    std::unique_ptr<VertexArray> quadVA;
    std::unique_ptr<VertexBuffer> quadVB;
    std::unique_ptr<IndexBuffer> quadIB;
    
    glm::mat4 projectionMatrix;
    int windowWidth = 1920;
    int windowHeight = 1080;

public:
    UIRenderer();
    ~UIRenderer();

    // Initialize the renderer (create shader, buffers)
    void init();

    // Render draw commands
    void render(const std::vector<UIDrawCommand>& commands);

    // Set projection matrix for screen space rendering
    void setProjectionMatrix(const glm::mat4& proj);
    void updateWindowSize(int width, int height);

private:
    void createShader();
    void createQuadGeometry();
    void drawRectangle(const UIRect& rect, const glm::vec4& color);
};
