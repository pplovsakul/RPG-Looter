#include "GUIRenderer.h"
#include <iostream>
#include <vector>

namespace CustomGUI {

// Vertex shader source
static const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;

uniform mat4 uProjection;

out vec2 vTexCoord;
out vec4 vColor;

void main() {
    vTexCoord = aTexCoord;
    vColor = aColor;
    gl_Position = uProjection * vec4(aPosition, 0.0, 1.0);
}
)";

// Fragment shader source
static const char* fragmentShaderSource = R"(
#version 330 core

in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uTexture;

out vec4 FragColor;

void main() {
    FragColor = vColor * texture(uTexture, vTexCoord);
}
)";

GUIRenderer::GUIRenderer()
    : shaderProgram(0), vao(0), vbo(0), ebo(0), 
      projMatrixLoc(-1), textureLoc(-1), fontTextureId(0),
      displayWidth(800), displayHeight(600) {
}

GUIRenderer::~GUIRenderer() {
    shutdown();
}

bool GUIRenderer::init() {
    // Create shader program
    if (!createShaderProgram()) {
        std::cerr << "Failed to create shader program" << std::endl;
        return false;
    }
    
    // Get uniform locations
    projMatrixLoc = glGetUniformLocation(shaderProgram, "uProjection");
    textureLoc = glGetUniformLocation(shaderProgram, "uTexture");
    
    // Create buffers
    createBuffers();
    
    // Create font texture
    createFontTexture();
    
    return true;
}

void GUIRenderer::shutdown() {
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    
    if (vbo) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    
    if (ebo) {
        glDeleteBuffers(1, &ebo);
        ebo = 0;
    }
    
    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
    
    if (fontTextureId) {
        glDeleteTextures(1, &fontTextureId);
        fontTextureId = 0;
    }
}

void GUIRenderer::setDisplaySize(int width, int height) {
    displayWidth = width;
    displayHeight = height;
}

void GUIRenderer::render(const GUIDrawList& drawList) {
    if (drawList.getCommandCount() == 0)
        return;
    
    // Save OpenGL state
    GLint lastProgram, lastTexture, lastVAO, lastVBO, lastEBO;
    GLint lastViewport[4];
    GLint lastScissorBox[4];
    GLboolean lastEnableBlend, lastEnableCullFace, lastEnableDepthTest, lastEnableScissorTest;
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVAO);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastVBO);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &lastEBO);
    glGetIntegerv(GL_VIEWPORT, lastViewport);
    glGetIntegerv(GL_SCISSOR_BOX, lastScissorBox);
    lastEnableBlend = glIsEnabled(GL_BLEND);
    lastEnableCullFace = glIsEnabled(GL_CULL_FACE);
    lastEnableDepthTest = glIsEnabled(GL_DEPTH_TEST);
    lastEnableScissorTest = glIsEnabled(GL_SCISSOR_TEST);
    
    // Setup render state
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    
    // Setup viewport
    glViewport(0, 0, displayWidth, displayHeight);
    
    // Setup orthographic projection matrix
    float L = 0.0f;
    float R = static_cast<float>(displayWidth);
    float T = 0.0f;
    float B = static_cast<float>(displayHeight);
    
    const float orthoProjection[4][4] = {
        { 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
        { 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
        { 0.0f,         0.0f,        -1.0f,   0.0f },
        { (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
    };
    
    // Use shader program
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &orthoProjection[0][0]);
    glUniform1i(textureLoc, 0);
    
    // Bind VAO
    glBindVertexArray(vao);
    
    // Upload vertex and index data
    const auto& vertices = drawList.getVertices();
    const auto& indices = drawList.getIndices();
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GUIVertex), 
                 vertices.data(), GL_STREAM_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), 
                 indices.data(), GL_STREAM_DRAW);
    
    // Render command lists
    const auto& commands = drawList.getCommands();
    unsigned int idxOffset = 0;
    
    for (const auto& cmd : commands) {
        if (cmd.elemCount == 0)
            continue;
        
        // Apply scissor/clipping rectangle
        glScissor(
            static_cast<int>(cmd.clipRect.min.x),
            static_cast<int>(displayHeight - cmd.clipRect.max.y),
            static_cast<int>(cmd.clipRect.max.x - cmd.clipRect.min.x),
            static_cast<int>(cmd.clipRect.max.y - cmd.clipRect.min.y)
        );
        
        // Bind texture
        GLuint texId = cmd.textureId ? cmd.textureId : fontTextureId;
        glBindTexture(GL_TEXTURE_2D, texId);
        
        // Draw
        glDrawElements(GL_TRIANGLES, cmd.elemCount, GL_UNSIGNED_INT, 
                      reinterpret_cast<void*>(idxOffset * sizeof(unsigned int)));
        
        idxOffset += cmd.elemCount;
    }
    
    // Restore OpenGL state
    glUseProgram(lastProgram);
    glBindTexture(GL_TEXTURE_2D, lastTexture);
    glBindVertexArray(lastVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lastVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lastEBO);
    glViewport(lastViewport[0], lastViewport[1], lastViewport[2], lastViewport[3]);
    glScissor(lastScissorBox[0], lastScissorBox[1], lastScissorBox[2], lastScissorBox[3]);
    
    if (lastEnableBlend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (lastEnableCullFace) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (lastEnableDepthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (lastEnableScissorTest) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
}

bool GUIRenderer::createShaderProgram() {
    // Compile shaders
    GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    if (!vertShader || !fragShader) {
        if (vertShader) glDeleteShader(vertShader);
        if (fragShader) glDeleteShader(fragShader);
        return false;
    }
    
    // Create program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    
    bool success = linkProgram(shaderProgram);
    
    // Cleanup shaders
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    
    return success;
}

void GUIRenderer::createBuffers() {
    // Create VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // Create VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    // Create EBO
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    
    // Setup vertex attributes
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GUIVertex), 
                         reinterpret_cast<void*>(offsetof(GUIVertex, pos)));
    
    // TexCoord
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GUIVertex), 
                         reinterpret_cast<void*>(offsetof(GUIVertex, uv)));
    
    // Color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(GUIVertex), 
                         reinterpret_cast<void*>(offsetof(GUIVertex, col)));
    
    glBindVertexArray(0);
}

void GUIRenderer::createFontTexture() {
    // Create a simple white texture for now
    unsigned char pixels[4] = { 255, 255, 255, 255 };
    
    glGenTextures(1, &fontTextureId);
    glBindTexture(GL_TEXTURE_2D, fontTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

GLuint GUIRenderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

bool GUIRenderer::linkProgram(GLuint program) {
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return false;
    }
    
    return true;
}

} // namespace CustomGUI
