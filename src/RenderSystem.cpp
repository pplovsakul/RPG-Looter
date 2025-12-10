#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "RenderSystem.h"
#include "Components.h"
#include "Entity.h"
#include "Shader.h"           
#include "Texture.h"          
#include "VertexArray.h"      
#include "VertexBuffer.h"     
#include "IndexBuffer.h"      
#include "VertexBufferLayout.h"
#include "vendor/glm/gtc/matrix_transform.hpp"
#include <algorithm>
#include <vector>            
#include <cmath>


RenderSystem::RenderSystem() {
    assetManager = AssetManager::getInstance();
    viewMatrix = glm::mat4(1.0f);
    // Perspective projection for 3D rendering
    projectionMatrix = glm::perspective(glm::radians(60.0f), 1920.0f / 1080.0f, 0.1f, 1000.0f);
}

void RenderSystem::init() {
    createDefaultAssets();
    // GL state for 3D rendering
    glViewport(0, 0, 1920, 1080);
    glEnable(GL_DEPTH_TEST);           // Enable depth testing
    glDepthFunc(GL_LESS);              // Accept fragment if closer
    glEnable(GL_CULL_FACE);            // Enable backface culling
    glCullFace(GL_BACK);               // Cull back faces
    glFrontFace(GL_CCW);               // Counter-clockwise winding is front
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // White background for better model visibility
    viewMatrix = glm::mat4(1.0f);
}

void RenderSystem::createDefaultAssets() {
    // Cube Mesh for 3D rendering (with normals)
    // Format: position (3) + normal (3) + texcoord (2)
    float cubeVertices[] = {
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        // Left face
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        // Right face
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        // Top face
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    };

    unsigned int cubeIndices[] = {
        0, 1, 2, 2, 3, 0,       // Back
        4, 5, 6, 6, 7, 4,       // Front
        8, 9, 10, 10, 11, 8,    // Left
        12, 13, 14, 14, 15, 12, // Right
        16, 17, 18, 18, 19, 16, // Bottom
        20, 21, 22, 22, 23, 20  // Top
    };

    assetManager->createMesh("cube", cubeVertices, sizeof(cubeVertices),
        cubeIndices, 36);

    std::cout << "[RenderSystem] Created 3D cube mesh\n";

    // Shader laden
    Shader* loaded = assetManager->loadShader("default", "res/ECS_shaders");
    if (!loaded) {
        std::cout << "[RenderSystem] Primary shader path failed, trying res/shaders/ECS.shader...\n";
        loaded = assetManager->loadShader("default", "res/shaders/ECS.shader");
    } else {
        std::cout << "[RenderSystem] Shader loaded from res/ECS_shaders\n";
    }

    // Fallback: Create 3D shader
    std::cout << "[RenderSystem] Creating fallback 3D shader...\n";
    createFallbackShader();
}

void RenderSystem::update(EntityManager& em, float dt) {
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Find active camera
    Entity* activeCamera = nullptr;
    glm::vec3 cameraPos(0.0f);
    auto cameraEntities = em.getEntitiesWith<CameraComponent>();
    for (Entity* e : cameraEntities) {
        auto cam = e->getComponent<CameraComponent>();
        if (cam && cam->isActive) {
            activeCamera = e;
            auto transform = e->getComponent<TransformComponent>();
            if (transform) {
                cameraPos = transform->position;
                // Update camera vectors based on rotation
                cam->updateVectors(transform->rotation);
                // Set view matrix
                viewMatrix = cam->getViewMatrix(transform);
            }
            break;
        }
    }
    
    // If no camera found, use default view
    if (!activeCamera) {
        viewMatrix = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 5.0f),  // Camera position
            glm::vec3(0.0f, 0.0f, 0.0f),  // Look at origin
            glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
        );
        cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    }
    
    // Render all entities with ModelComponent
    auto modelEntities = em.getEntitiesWith<ModelComponent>();
    
    // Debug output (only when entity count changes)
    static size_t lastModelEntityCount = 0;
    if (modelEntities.size() != lastModelEntityCount) {
        std::cout << "[RenderSystem] Now rendering " << modelEntities.size() 
                  << " entities with ModelComponent\n";
        for (Entity* e : modelEntities) {
            auto model = e->getComponent<ModelComponent>();
            if (model) {
                std::cout << "  - Entity [" << e->id << "] " << e->tag 
                          << ": " << model->meshes.size() << " meshes\n";
            }
        }
        lastModelEntityCount = modelEntities.size();
    }
    
    for (Entity* e : modelEntities) {
        if (!e || !e->active) continue;
        
        auto model = e->getComponent<ModelComponent>();
        auto transform = e->getComponent<TransformComponent>();
        
        if (!model || !transform) continue;
        
        // Build model matrix
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, transform->position);
        
        // Apply rotation (Euler angles: pitch, yaw, roll)
        modelMatrix = glm::rotate(modelMatrix, transform->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw
        modelMatrix = glm::rotate(modelMatrix, transform->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch
        modelMatrix = glm::rotate(modelMatrix, transform->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Roll
        
        modelMatrix = glm::scale(modelMatrix, transform->scale);
        
        // Render each mesh in the model
        for (const auto& mesh : model->meshes) {
            renderMesh(mesh, modelMatrix, cameraPos);
        }
    }
    
    // Also render simple RenderComponent entities (for backward compatibility with cube mesh)
    auto renderEntities = em.getEntitiesWith<RenderComponent>();
    for (Entity* e : renderEntities) {
        if (!e || !e->active) continue;
        if (e->hasComponent<ModelComponent>()) continue; // Skip if has ModelComponent
        
        auto render = e->getComponent<RenderComponent>();
        auto transform = e->getComponent<TransformComponent>();
        
        if (!render || !transform || !render->enabled) continue;
        
        // Get mesh
        VertexArray* mesh = assetManager->getMesh(render->meshName);
        if (!mesh) continue;
        
        // Build model matrix
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, transform->position);
        modelMatrix = glm::rotate(modelMatrix, transform->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, transform->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, transform->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, transform->scale);
        
        // Use fallback shader
        if (fallbackShaderID != 0) {
            glUseProgram(fallbackShaderID);
            
            // Set uniforms
            GLint modelLoc = glGetUniformLocation(fallbackShaderID, "u_Model");
            GLint viewLoc = glGetUniformLocation(fallbackShaderID, "u_View");
            GLint projLoc = glGetUniformLocation(fallbackShaderID, "u_Projection");
            GLint lightPosLoc = glGetUniformLocation(fallbackShaderID, "u_LightPos");
            GLint lightColorLoc = glGetUniformLocation(fallbackShaderID, "u_LightColor");
            GLint ambientLoc = glGetUniformLocation(fallbackShaderID, "u_AmbientColor");
            GLint viewPosLoc = glGetUniformLocation(fallbackShaderID, "u_ViewPos");
            GLint objectColorLoc = glGetUniformLocation(fallbackShaderID, "u_ObjectColor");
            GLint useTexLoc = glGetUniformLocation(fallbackShaderID, "u_UseTexture");
            
            if (modelLoc != -1) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelMatrix[0][0]);
            if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &viewMatrix[0][0]);
            if (projLoc != -1) glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projectionMatrix[0][0]);
            if (lightPosLoc != -1) glUniform3fv(lightPosLoc, 1, &lightPos[0]);
            if (lightColorLoc != -1) glUniform3fv(lightColorLoc, 1, &lightColor[0]);
            if (ambientLoc != -1) glUniform3fv(ambientLoc, 1, &ambientColor[0]);
            if (viewPosLoc != -1) glUniform3fv(viewPosLoc, 1, &cameraPos[0]);
            if (objectColorLoc != -1) glUniform3fv(objectColorLoc, 1, &render->color[0]);
            if (useTexLoc != -1) glUniform1i(useTexLoc, 0);
            
            // Draw
            mesh->Bind();
            glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
        }
    }
}

void RenderSystem::renderMesh(const ModelComponent::Mesh& mesh, const glm::mat4& modelMatrix, 
                              const glm::vec3& cameraPos) {
    if (mesh.vertices.empty() || mesh.indices.empty()) return;
    
    // Initialize buffers on first use (cached in the mesh)
    if (!mesh.buffersInitialized) {
        glGenVertexArrays(1, &mesh.VAO);
        glGenBuffers(1, &mesh.VBO);
        glGenBuffers(1, &mesh.EBO);
        
        glBindVertexArray(mesh.VAO);
        
        // Interleave vertex data: position (3) + normal (3) + uv (2)
        std::vector<float> vertexData;
        vertexData.reserve(mesh.vertices.size() * 8);
        
        const glm::vec3 DEFAULT_NORMAL(0.0f, 1.0f, 0.0f);
        const glm::vec2 DEFAULT_UV(0.0f, 0.0f);
        
        for (size_t i = 0; i < mesh.vertices.size(); i++) {
            // Position
            vertexData.push_back(mesh.vertices[i].x);
            vertexData.push_back(mesh.vertices[i].y);
            vertexData.push_back(mesh.vertices[i].z);
            
            // Normal
            const glm::vec3& normal = (i < mesh.normals.size()) ? mesh.normals[i] : DEFAULT_NORMAL;
            vertexData.push_back(normal.x);
            vertexData.push_back(normal.y);
            vertexData.push_back(normal.z);
            
            // UV
            const glm::vec2& uv = (i < mesh.uvs.size()) ? mesh.uvs[i] : DEFAULT_UV;
            vertexData.push_back(uv.x);
            vertexData.push_back(uv.y);
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), 
                     mesh.indices.data(), GL_STATIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // UV attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        
        mesh.buffersInitialized = true;
    } else {
        // Use cached buffers
        glBindVertexArray(mesh.VAO);
    }
    
    // Use shader
    if (fallbackShaderID != 0) {
        glUseProgram(fallbackShaderID);
        
        // Set uniforms
        GLint modelLoc = glGetUniformLocation(fallbackShaderID, "u_Model");
        GLint viewLoc = glGetUniformLocation(fallbackShaderID, "u_View");
        GLint projLoc = glGetUniformLocation(fallbackShaderID, "u_Projection");
        GLint lightPosLoc = glGetUniformLocation(fallbackShaderID, "u_LightPos");
        GLint lightColorLoc = glGetUniformLocation(fallbackShaderID, "u_LightColor");
        GLint ambientLoc = glGetUniformLocation(fallbackShaderID, "u_AmbientColor");
        GLint viewPosLoc = glGetUniformLocation(fallbackShaderID, "u_ViewPos");
        GLint objectColorLoc = glGetUniformLocation(fallbackShaderID, "u_ObjectColor");
        GLint useTexLoc = glGetUniformLocation(fallbackShaderID, "u_UseTexture");
        
        if (modelLoc != -1) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelMatrix[0][0]);
        if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &viewMatrix[0][0]);
        if (projLoc != -1) glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projectionMatrix[0][0]);
        if (lightPosLoc != -1) glUniform3fv(lightPosLoc, 1, &lightPos[0]);
        if (lightColorLoc != -1) glUniform3fv(lightColorLoc, 1, &lightColor[0]);
        if (ambientLoc != -1) glUniform3fv(ambientLoc, 1, &ambientColor[0]);
        if (viewPosLoc != -1) glUniform3fv(viewPosLoc, 1, &cameraPos[0]);
        if (objectColorLoc != -1) glUniform3fv(objectColorLoc, 1, &mesh.color[0]);
        if (useTexLoc != -1) glUniform1i(useTexLoc, 0);
        
        // Texture handling
        if (!mesh.textureName.empty()) {
            Texture* tex = assetManager->getTexture(mesh.textureName);
            if (tex) {
                glActiveTexture(GL_TEXTURE0);
                tex->Bind(0);
                if (useTexLoc != -1) glUniform1i(useTexLoc, 1);
            }
        }
        
        // Draw
        glDrawElements(GL_TRIANGLES, (GLsizei)mesh.indices.size(), GL_UNSIGNED_INT, 0);
    }
}

void RenderSystem::setViewMatrix(const glm::mat4& view) {
    viewMatrix = view;
}

void RenderSystem::setProjectionMatrix(const glm::mat4& proj) {
    projectionMatrix = proj;
}

void RenderSystem::createFallbackShader() {
    // 3D Vertex Shader with lighting
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 a_Position;
        layout (location = 1) in vec3 a_Normal;
        layout (location = 2) in vec2 a_TexCoord;
        
        uniform mat4 u_Model;
        uniform mat4 u_View;
        uniform mat4 u_Projection;
        
        out vec3 v_FragPos;
        out vec3 v_Normal;
        out vec2 v_TexCoord;
        
        void main() {
            v_FragPos = vec3(u_Model * vec4(a_Position, 1.0));
            v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
            v_TexCoord = a_TexCoord;
            gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
        }
    )";

    // Fragment Shader with Phong lighting
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 v_FragPos;
        in vec3 v_Normal;
        in vec2 v_TexCoord;
        
        uniform vec3 u_LightPos;
        uniform vec3 u_LightColor;
        uniform vec3 u_AmbientColor;
        uniform vec3 u_ViewPos;
        uniform vec3 u_ObjectColor;
        uniform sampler2D u_Texture;
        uniform int u_UseTexture;
        
        void main() {
            // Ambient
            vec3 ambient = u_AmbientColor;
            
            // Diffuse
            vec3 norm = normalize(v_Normal);
            vec3 lightDir = normalize(u_LightPos - v_FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * u_LightColor;
            
            // Specular
            vec3 viewDir = normalize(u_ViewPos - v_FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
            vec3 specular = 0.5 * spec * u_LightColor;
            
            vec3 result = (ambient + diffuse + specular) * u_ObjectColor;
            
            if (u_UseTexture == 1) {
                vec4 texColor = texture(u_Texture, v_TexCoord);
                FragColor = vec4(result, 1.0) * texColor;
            } else {
                FragColor = vec4(result, 1.0);
            }
        }
    )";

    // Shader kompilieren mit Error Checking
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    // Check vertex shader compilation
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "[RenderSystem] Vertex shader compilation failed: " << infoLog << "\n";
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "[RenderSystem] Fragment shader compilation failed: " << infoLog << "\n";
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // Check program linking
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "[RenderSystem] Shader program linking failed: " << infoLog << "\n";
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create and store as AssetManager shader for consistency
    try {
        std::cout << "[RenderSystem] Fallback 3D shader created with ID: " << shaderProgram << "\n";
        fallbackShaderID = shaderProgram;
        cachedProgramID = shaderProgram; // Cache the program ID
    } catch (const std::exception& e) {
        std::cout << "[RenderSystem] Failed to store fallback shader: " << e.what() << "\n";
    }
}
