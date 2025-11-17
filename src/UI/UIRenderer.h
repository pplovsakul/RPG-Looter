#pragma once
#include "UIWidget.h"
#include "../Shader.h"
#include "../VertexArray.h"
#include "../VertexBuffer.h"
#include "../IndexBuffer.h"
#include "../vendor/glm/glm.hpp"
#include <glad/glad.h>
#include <memory>
#include <vector>

class Font;

class UIRenderer {
private:
    std::unique_ptr<VertexArray> quadVA;
    std::unique_ptr<VertexBuffer> quadVB;
    std::unique_ptr<IndexBuffer> quadIB;
    
    GLuint uiShaderProgramID = 0;      // Colored rectangle shader
    GLuint textShaderProgramID = 0;    // SDF text shader
    
    // Text rendering buffers (dynamic)
    std::unique_ptr<VertexArray> textVA;
    std::unique_ptr<VertexBuffer> textVB;
    std::unique_ptr<IndexBuffer> textIB;
    
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
    
    // Add text to be rendered (returns updated draw commands)
    void addText(std::vector<UIDrawCommand>& commands, const std::string& text, 
                 float x, float y, Font* font, const glm::vec4& color, float scale = 1.0f);

private:
    void createUIShader();
    void createTextShader();
    void createQuadGeometry();
    void createTextGeometry();
    void drawRectangle(const UIRect& rect, const glm::vec4& color);
    void drawText(const std::string& text, float x, float y, Font* font, 
                  const glm::vec4& color, float scale);
};
