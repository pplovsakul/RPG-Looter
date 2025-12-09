#pragma once

#include "GUIDrawList.h"
#include "GUITypes.h"
#include <glad/glad.h>
#include <memory>

namespace CustomGUI {

class GUIRenderer {
private:
    // OpenGL objects
    GLuint shaderProgram;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    
    // Shader uniforms
    GLint projMatrixLoc;
    GLint textureLoc;
    
    // Font texture (white pixel for now)
    GLuint fontTextureId;
    
    // Display size
    int displayWidth;
    int displayHeight;
    
public:
    GUIRenderer();
    ~GUIRenderer();
    
    // Initialize the renderer
    bool init();
    
    // Cleanup resources
    void shutdown();
    
    // Set display size
    void setDisplaySize(int width, int height);
    
    // Render a draw list
    void render(const GUIDrawList& drawList);
    
private:
    // Create shader program
    bool createShaderProgram();
    
    // Create buffers
    void createBuffers();
    
    // Create font texture
    void createFontTexture();
    
    // Helper to compile shader
    GLuint compileShader(GLenum type, const char* source);
    
    // Helper to link program
    bool linkProgram(GLuint program);
};

} // namespace CustomGUI
