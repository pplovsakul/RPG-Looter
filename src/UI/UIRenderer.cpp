#include "UIRenderer.h"
#include "../VertexBufferLayout.h"
#include "../Font.h"
#include "../Texture.h"
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include <glad/glad.h>
#include <iostream>

UIRenderer::UIRenderer() {
    std::cout << "[UIRenderer] Created" << std::endl;
}

UIRenderer::~UIRenderer() {
    if (uiShaderProgramID != 0) {
        glDeleteProgram(uiShaderProgramID);
        uiShaderProgramID = 0;
    }
    if (textShaderProgramID != 0) {
        glDeleteProgram(textShaderProgramID);
        textShaderProgramID = 0;
    }
    std::cout << "[UIRenderer] Destroyed" << std::endl;
}

void UIRenderer::init() {
    createUIShader();
    createTextShader();
    createQuadGeometry();
    createTextGeometry();
    
    // Set default orthographic projection for screen space (origin top-left)
    updateWindowSize(windowWidth, windowHeight);
    
    std::cout << "[UIRenderer] Initialized" << std::endl;
}

void UIRenderer::createUIShader() {
    // Create a simple shader for colored rectangles
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec2 a_Position;
        
        uniform mat4 u_Projection;
        uniform mat4 u_Model;
        
        void main() {
            gl_Position = u_Projection * u_Model * vec4(a_Position, 0.0, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        layout(location = 0) out vec4 color;
        
        uniform vec4 u_Color;
        
        void main() {
            color = u_Color;
        }
    )";

    // For now, we'll create the shader inline
    // In a real implementation, you might want to use the existing Shader class
    // but it expects a file path, so we create it manually here
    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    // Check vertex shader compilation
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "[UIRenderer] Vertex shader compilation failed: " << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "[UIRenderer] Fragment shader compilation failed: " << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // Check linking
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "[UIRenderer] Shader program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Store the program ID for use in rendering
    uiShaderProgramID = shaderProgram;
    
    std::cout << "[UIRenderer] UI Shader created (Program ID: " << uiShaderProgramID << ")" << std::endl;
}

void UIRenderer::createQuadGeometry() {
    // Create a unit quad (0,0) to (1,1) that we'll scale and position
    float vertices[] = {
        0.0f, 0.0f,  // bottom-left
        1.0f, 0.0f,  // bottom-right
        1.0f, 1.0f,  // top-right
        0.0f, 1.0f   // top-left
    };

    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };

    quadVA = std::make_unique<VertexArray>();
    quadVB = std::make_unique<VertexBuffer>(vertices, sizeof(vertices));
    
    VertexBufferLayout layout;
    layout.AddFloat(2); // position (x, y)
    quadVA->AddBuffer(*quadVB, layout);
    
    quadIB = std::make_unique<IndexBuffer>(indices, 6);
    
    std::cout << "[UIRenderer] Quad geometry created" << std::endl;
}

void UIRenderer::createTextShader() {
    // Create SDF text shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec2 a_Position;
        layout(location = 1) in vec2 a_TexCoord;
        
        uniform mat4 u_Projection;
        uniform mat4 u_Model;
        
        out vec2 v_TexCoord;
        
        void main() {
            v_TexCoord = a_TexCoord;
            gl_Position = u_Projection * u_Model * vec4(a_Position, 0.0, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        layout(location = 0) out vec4 color;
        
        in vec2 v_TexCoord;
        
        uniform sampler2D u_Texture;
        uniform vec4 u_Color;
        uniform float u_DistanceRange;
        
        void main() {
            // Sample the SDF distance from the texture
            float distance = texture(u_Texture, v_TexCoord).a;
            
            // Simple SDF rendering (can be improved with better edge antialiasing)
            float alpha = smoothstep(0.5 - u_DistanceRange/255.0, 0.5 + u_DistanceRange/255.0, distance);
            
            color = vec4(u_Color.rgb, u_Color.a * alpha);
        }
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "[UIRenderer] Text vertex shader compilation failed: " << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "[UIRenderer] Text fragment shader compilation failed: " << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "[UIRenderer] Text shader program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    textShaderProgramID = shaderProgram;
    
    std::cout << "[UIRenderer] Text Shader created (Program ID: " << textShaderProgramID << ")" << std::endl;
}

void UIRenderer::createTextGeometry() {
    // Create buffers for dynamic text rendering
    // Initialize with a small dummy size (we'll recreate as needed)
    float dummyVertices[16] = {0}; // 4 vertices * 4 floats (pos + uv)
    unsigned int dummyIndices[6] = {0, 1, 2, 2, 3, 0};
    
    textVA = std::make_unique<VertexArray>();
    textVB = std::make_unique<VertexBuffer>(dummyVertices, sizeof(dummyVertices));
    textIB = std::make_unique<IndexBuffer>(dummyIndices, 6);
    
    VertexBufferLayout layout;
    layout.AddFloat(2); // position (x, y)
    layout.AddFloat(2); // texcoord (u, v)
    textVA->AddBuffer(*textVB, layout);
    
    std::cout << "[UIRenderer] Text geometry buffers created" << std::endl;
}

void UIRenderer::render(const std::vector<UIDrawCommand>& commands) {
    if (commands.empty()) {
        return;
    }

    // Enable blending for UI transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto& cmd : commands) {
        if (cmd.type == UIDrawCommand::Type::Rectangle) {
            drawRectangle(cmd.rect, cmd.color);
        } else if (cmd.type == UIDrawCommand::Type::Text) {
            if (cmd.font) {
                drawText(cmd.text, cmd.rect.x, cmd.rect.y, cmd.font, cmd.color, cmd.textScale);
            }
        }
    }

    glDisable(GL_BLEND);
}

void UIRenderer::drawRectangle(const UIRect& rect, const glm::vec4& color) {
    if (uiShaderProgramID == 0 || !quadVA || !quadIB) {
        return;
    }

    // Create model matrix for this rectangle
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(rect.x, rect.y, 0.0f));
    model = glm::scale(model, glm::vec3(rect.width, rect.height, 1.0f));

    // Use the shader program
    glUseProgram(uiShaderProgramID);
    
    // Set uniforms
    GLint projLoc = glGetUniformLocation(uiShaderProgramID, "u_Projection");
    GLint modelLoc = glGetUniformLocation(uiShaderProgramID, "u_Model");
    GLint colorLoc = glGetUniformLocation(uiShaderProgramID, "u_Color");
    
    if (projLoc != -1) {
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projectionMatrix[0][0]);
    }
    if (modelLoc != -1) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    }
    if (colorLoc != -1) {
        glUniform4f(colorLoc, color.r, color.g, color.b, color.a);
    }
    
    // Bind and draw
    quadVA->Bind();
    quadIB->Bind();
    
    glDrawElements(GL_TRIANGLES, quadIB->GetCount(), GL_UNSIGNED_INT, nullptr);
    
    // Unbind
    glUseProgram(0);
}

void UIRenderer::drawText(const std::string& text, float x, float y, Font* font, 
                          const glm::vec4& color, float scale) {
    if (!font || text.empty() || textShaderProgramID == 0) {
        return;
    }
    
    // Build geometry for the text
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    float cursorX = x;
    float cursorY = y;
    
    const float atlasWidth = static_cast<float>(font->getAtlasWidth());
    const float atlasHeight = static_cast<float>(font->getAtlasHeight());
    
    unsigned int vertexOffset = 0;
    
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        
        // Handle newlines
        if (c == '\n') {
            cursorX = x;
            cursorY += font->getLineHeight() * scale;
            continue;
        }
        
        const GlyphMetrics* glyph = font->getGlyph(static_cast<int>(c));
        if (!glyph) {
            continue; // Skip unknown glyphs
        }
        
        // Calculate glyph position and size
        float glyphWidth = (glyph->atlasBounds[2] - glyph->atlasBounds[0]);
        float glyphHeight = (glyph->atlasBounds[3] - glyph->atlasBounds[1]);
        
        float x0 = cursorX;
        float y0 = cursorY;
        float x1 = x0 + glyphWidth * scale;
        float y1 = y0 + glyphHeight * scale;
        
        // Calculate texture coordinates
        float u0 = glyph->atlasBounds[0] / atlasWidth;
        float v0 = glyph->atlasBounds[1] / atlasHeight;
        float u1 = glyph->atlasBounds[2] / atlasWidth;
        float v1 = glyph->atlasBounds[3] / atlasHeight;
        
        // Add vertices (position + texcoord)
        vertices.push_back(x0); vertices.push_back(y0); vertices.push_back(u0); vertices.push_back(v0);
        vertices.push_back(x1); vertices.push_back(y0); vertices.push_back(u1); vertices.push_back(v0);
        vertices.push_back(x1); vertices.push_back(y1); vertices.push_back(u1); vertices.push_back(v1);
        vertices.push_back(x0); vertices.push_back(y1); vertices.push_back(u0); vertices.push_back(v1);
        
        // Add indices
        indices.push_back(vertexOffset + 0);
        indices.push_back(vertexOffset + 1);
        indices.push_back(vertexOffset + 2);
        indices.push_back(vertexOffset + 2);
        indices.push_back(vertexOffset + 3);
        indices.push_back(vertexOffset + 0);
        
        vertexOffset += 4;
        
        // Advance cursor
        cursorX += glyph->advance * scale;
        
        // Apply kerning if available
        if (i + 1 < text.length()) {
            float kerning = font->getKerning(static_cast<int>(c), static_cast<int>(text[i + 1]));
            cursorX += kerning * scale;
        }
    }
    
    if (vertices.empty()) {
        return; // Nothing to render
    }
    
    // Recreate buffers with new geometry (simpler than trying to update)
    textVB = std::make_unique<VertexBuffer>(vertices.data(), vertices.size() * sizeof(float));
    textIB = std::make_unique<IndexBuffer>(indices.data(), indices.size());
    
    // Rebind to VAO
    VertexBufferLayout layout;
    layout.AddFloat(2); // position (x, y)
    layout.AddFloat(2); // texcoord (u, v)
    textVA->AddBuffer(*textVB, layout);
    
    // Use text shader
    glUseProgram(textShaderProgramID);
    
    // Set uniforms
    GLint projLoc = glGetUniformLocation(textShaderProgramID, "u_Projection");
    GLint modelLoc = glGetUniformLocation(textShaderProgramID, "u_Model");
    GLint colorLoc = glGetUniformLocation(textShaderProgramID, "u_Color");
    GLint texLoc = glGetUniformLocation(textShaderProgramID, "u_Texture");
    GLint distRangeLoc = glGetUniformLocation(textShaderProgramID, "u_DistanceRange");
    
    glm::mat4 model = glm::mat4(1.0f); // Identity - vertices already in screen space
    
    if (projLoc != -1) {
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projectionMatrix[0][0]);
    }
    if (modelLoc != -1) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    }
    if (colorLoc != -1) {
        glUniform4f(colorLoc, color.r, color.g, color.b, color.a);
    }
    if (texLoc != -1) {
        glUniform1i(texLoc, 0);
    }
    if (distRangeLoc != -1) {
        glUniform1f(distRangeLoc, static_cast<float>(font->getDistanceRange()));
    }
    
    // Bind font atlas texture
    if (font->getAtlasTexture()) {
        font->getAtlasTexture()->Bind(0);
    }
    
    // Draw
    textVA->Bind();
    textIB->Bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    
    // Unbind
    glUseProgram(0);
}

void UIRenderer::addText(std::vector<UIDrawCommand>& commands, const std::string& text, 
                         float x, float y, Font* font, const glm::vec4& color, float scale) {
    UIDrawCommand cmd;
    cmd.type = UIDrawCommand::Type::Text;
    cmd.rect = UIRect(x, y, 0, 0); // Width/height not used for text
    cmd.text = text;
    cmd.font = font;
    cmd.color = color;
    cmd.textScale = scale;
    commands.push_back(cmd);
}

void UIRenderer::setProjectionMatrix(const glm::mat4& proj) {
    projectionMatrix = proj;
}

void UIRenderer::updateWindowSize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    
    // Create orthographic projection: origin at top-left, +Y goes down
    projectionMatrix = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
    
    std::cout << "[UIRenderer] Window size updated: " << width << "x" << height << std::endl;
}
