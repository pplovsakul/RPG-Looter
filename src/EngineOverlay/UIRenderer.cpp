#include "UIRenderer.h"
#include "../Debug.h"
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include <iostream>

namespace EngineUI {

static const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

uniform mat4 uProjection;

out vec2 vUV;
out vec4 vColor;

void main() {
    vUV = aUV;
    vColor = aColor;
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

static const char* fragmentShaderSource = R"(
#version 330 core
in vec2 vUV;
in vec4 vColor;

uniform sampler2D uTexture;

out vec4 FragColor;

void main() {
    // Always sample texture and multiply by vertex color
    // For solid color rendering, texture will be 1x1 white pixel
    vec4 texColor = texture(uTexture, vUV);
    FragColor = vColor * texColor;
}
)";

UIRenderer::UIRenderer() {
}

UIRenderer::~UIRenderer() {
    shutdown();
}

bool UIRenderer::createShaderProgram() {
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        return false;
    }
    
    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    // Link shader program
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);
    
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
        return false;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Get uniform locations
    m_uniformTexture = glGetUniformLocation(m_shaderProgram, "uTexture");
    m_uniformProjection = glGetUniformLocation(m_shaderProgram, "uProjection");
    
    return true;
}

bool UIRenderer::init(int screenWidth, int screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    
    // Create shader program
    if (!createShaderProgram()) {
        return false;
    }
    
    // Create VAO
    GLCall(glGenVertexArrays(1, &m_vao));
    GLCall(glBindVertexArray(m_vao));
    
    // Create VBO
    GLCall(glGenBuffers(1, &m_vbo));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    
    // Create EBO
    GLCall(glGenBuffers(1, &m_ebo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo));
    
    // Setup vertex attributes
    // Position
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(DrawVertex), 
           (void*)offsetof(DrawVertex, pos)));
    
    // UV
    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(DrawVertex), 
           (void*)offsetof(DrawVertex, uv)));
    
    // Color
    GLCall(glEnableVertexAttribArray(2));
    GLCall(glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(DrawVertex), 
           (void*)offsetof(DrawVertex, color)));
    
    GLCall(glBindVertexArray(0));
    
    // Create a 1x1 white texture for solid color rendering
    unsigned char whitePixel[4] = {255, 255, 255, 255};
    GLCall(glGenTextures(1, &m_fontTextureID));
    GLCall(glBindTexture(GL_TEXTURE_2D, m_fontTextureID));
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    return true;
}

void UIRenderer::shutdown() {
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_ebo) {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
    }
    if (m_shaderProgram) {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
    if (m_fontTextureID) {
        glDeleteTextures(1, &m_fontTextureID);
        m_fontTextureID = 0;
    }
}

void UIRenderer::setupRenderState(int screenWidth, int screenHeight) {
    // Disable depth test and culling for UI rendering
    GLCall(glDisable(GL_DEPTH_TEST));
    GLCall(glDisable(GL_CULL_FACE));
    
    // Enable blending with proper alpha handling
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendEquation(GL_FUNC_ADD));
    GLCall(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
    
    // Enable scissor test for clipping
    GLCall(glEnable(GL_SCISSOR_TEST));
    
    // Setup viewport
    GLCall(glViewport(0, 0, screenWidth, screenHeight));
    
    // Setup orthographic projection
    GLCall(glUseProgram(m_shaderProgram));
    
    glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
    GLCall(glUniformMatrix4fv(m_uniformProjection, 1, GL_FALSE, &projection[0][0]));
    
    // Bind texture unit
    GLCall(glUniform1i(m_uniformTexture, 0));
    GLCall(glActiveTexture(GL_TEXTURE0));
}

void UIRenderer::beginFrame(int screenWidth, int screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
}

void UIRenderer::endFrame() {
    // Nothing to do here for now
}

void UIRenderer::render(const DrawList& drawList) {
    const auto& vertices = drawList.getVertices();
    const auto& indices = drawList.getIndices();
    const auto& commands = drawList.getCommands();
    
    if (vertices.empty() || indices.empty() || commands.empty()) {
        return;
    }
    
    // Save GL state to restore later
    GLboolean lastEnableBlend = glIsEnabled(GL_BLEND);
    GLboolean lastEnableDepthTest = glIsEnabled(GL_DEPTH_TEST);
    GLboolean lastEnableCullFace = glIsEnabled(GL_CULL_FACE);
    GLboolean lastEnableScissorTest = glIsEnabled(GL_SCISSOR_TEST);
    GLint lastBlendSrcRGB, lastBlendDstRGB, lastBlendSrcAlpha, lastBlendDstAlpha;
    glGetIntegerv(GL_BLEND_SRC_RGB, &lastBlendSrcRGB);
    glGetIntegerv(GL_BLEND_DST_RGB, &lastBlendDstRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &lastBlendSrcAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &lastBlendDstAlpha);
    GLint lastViewport[4];
    glGetIntegerv(GL_VIEWPORT, lastViewport);
    
    setupRenderState(m_screenWidth, m_screenHeight);
    
    // Upload vertex data
    GLCall(glBindVertexArray(m_vao));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(DrawVertex), 
           vertices.data(), GL_STREAM_DRAW));
    
    // Upload index data
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), 
           indices.data(), GL_STREAM_DRAW));
    
    // Execute draw commands
    for (const auto& cmd : commands) {
        // Set clip rectangle
        if (cmd.clipRect.w > 0 && cmd.clipRect.h > 0) {
            int clipX = (int)cmd.clipRect.x;
            int clipY = (int)(m_screenHeight - cmd.clipRect.y - cmd.clipRect.h);
            int clipW = (int)cmd.clipRect.w;
            int clipH = (int)cmd.clipRect.h;
            GLCall(glScissor(clipX, clipY, clipW, clipH));
        } else {
            GLCall(glScissor(0, 0, m_screenWidth, m_screenHeight));
        }
        
        // Bind texture
        unsigned int texID = cmd.textureID ? cmd.textureID : m_fontTextureID;
        GLCall(glBindTexture(GL_TEXTURE_2D, texID));
        
        // Draw
        GLCall(glDrawElements(GL_TRIANGLES, cmd.elemCount, GL_UNSIGNED_INT, 
               (void*)(cmd.idxOffset * sizeof(unsigned int))));
    }
    
    // Restore GL state
    GLCall(glBindVertexArray(0));
    GLCall(glUseProgram(0));
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    
    if (lastEnableBlend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (lastEnableDepthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (lastEnableCullFace) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (lastEnableScissorTest) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    
    glBlendFuncSeparate(lastBlendSrcRGB, lastBlendDstRGB, lastBlendSrcAlpha, lastBlendDstAlpha);
    glViewport(lastViewport[0], lastViewport[1], lastViewport[2], lastViewport[3]);
}

} // namespace EngineUI
