#pragma once

#include "DrawList.h"
#include <glad/glad.h>

namespace EngineUI {

/**
 * UIRenderer - Renders DrawList commands using OpenGL
 */
class UIRenderer {
public:
    UIRenderer();
    ~UIRenderer();
    
    bool init(int screenWidth, int screenHeight);
    void shutdown();
    
    void beginFrame(int screenWidth, int screenHeight);
    void endFrame();
    
    void render(const DrawList& drawList);
    
    // Font texture management
    void setFontTexture(unsigned int textureID) { m_fontTextureID = textureID; }
    unsigned int getFontTexture() const { return m_fontTextureID; }
    
private:
    void setupRenderState(int screenWidth, int screenHeight);
    
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    GLuint m_shaderProgram = 0;
    GLint m_uniformTexture = -1;
    GLint m_uniformProjection = -1;
    
    unsigned int m_fontTextureID = 0;
    
    int m_screenWidth = 0;
    int m_screenHeight = 0;
    
    bool createShaderProgram();
};

} // namespace EngineUI
