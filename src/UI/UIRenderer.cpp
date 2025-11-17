#include "UIRenderer.h"
#include "../VertexBufferLayout.h"
#include <glad/glad.h>
#include <iostream>

UIRenderer::UIRenderer() {
    std::cout << "[UIRenderer] Created" << std::endl;
}

UIRenderer::~UIRenderer() {
    std::cout << "[UIRenderer] Destroyed" << std::endl;
}

void UIRenderer::init() {
    createShader();
    createQuadGeometry();
    
    // Set default orthographic projection for screen space (origin top-left)
    updateWindowSize(windowWidth, windowHeight);
    
    std::cout << "[UIRenderer] Initialized" << std::endl;
}

void UIRenderer::createShader() {
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

    // Create a Shader wrapper (we'll need to adapt this to work with raw GL program)
    // For now, store the program ID directly
    // Note: The existing Shader class might need adaptation, or we create a minimal wrapper
    uiShader = std::make_unique<Shader>();
    // We'll directly use the shader program ID in the render function
    
    std::cout << "[UIRenderer] UI Shader created (Program ID: " << shaderProgram << ")" << std::endl;
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
    layout.Push<float>(2); // position (x, y)
    quadVA->AddBuffer(*quadVB, layout);
    
    quadIB = std::make_unique<IndexBuffer>(indices, 6);
    
    std::cout << "[UIRenderer] Quad geometry created" << std::endl;
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
        }
        // Text rendering would be implemented here
    }

    glDisable(GL_BLEND);
}

void UIRenderer::drawRectangle(const UIRect& rect, const glm::vec4& color) {
    if (!uiShader || !quadVA || !quadIB) {
        return;
    }

    // Create model matrix for this rectangle
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(rect.x, rect.y, 0.0f));
    model = glm::scale(model, glm::vec3(rect.width, rect.height, 1.0f));

    // Note: Since we created the shader manually, we need to use it directly
    // This is a simplified approach - ideally we'd integrate with the existing Shader class
    // For now, this serves as a placeholder showing the architecture
    
    // Bind shader, set uniforms, and draw
    // This would use uiShader->Bind() and uiShader->SetUniform...() in a full implementation
    
    quadVA->Bind();
    quadIB->Bind();
    
    // In a complete implementation, we'd set uniforms here:
    // uiShader->SetUniformMat4f("u_Projection", projectionMatrix);
    // uiShader->SetUniformMat4f("u_Model", model);
    // uiShader->SetUniform4f("u_Color", color.r, color.g, color.b, color.a);
    
    glDrawElements(GL_TRIANGLES, quadIB->GetCount(), GL_UNSIGNED_INT, nullptr);
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
