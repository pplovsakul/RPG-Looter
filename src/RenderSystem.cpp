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
    // Projektionsmatrix für Bildschirm-Koordinaten (0-1920, 0-1080)
    // Consistent with main.cpp coordinate system
    projectionMatrix = glm::ortho(0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f, 1.0f);
}

void RenderSystem::init() {
    createDefaultAssets();
    // GL state for 2D rendering
    glViewport(0, 0, 1920, 1080); // Match window size; adjust if window size differs
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f); // dark background so white paddle is visible
    viewMatrix = glm::mat4(1.0f);
}

void RenderSystem::createDefaultAssets() {
    // Quad Mesh
    float quadVertices[] = {
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f
    };

    unsigned int quadIndices[6] = { 0, 1, 2, 2, 3, 0 };

    assetManager->createMesh("quad", quadVertices, sizeof(quadVertices),
        quadIndices, 6);

    // Triangle Mesh (use same local corners as CollisionUtils triangle: indices 0,1,3)
    float triVertices[] = {
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // corner 0
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // corner 1
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // corner 3
    };
    unsigned int triIndices[] = { 0, 1, 2 };
    assetManager->createMesh("triangle", triVertices, sizeof(triVertices), triIndices, 3);

    // Circle Mesh
    const int segments = 32;
    std::vector<float> circleVertices;
    std::vector<unsigned int> circleIndices;

    circleVertices.push_back(0.0f);
    circleVertices.push_back(0.0f);
    circleVertices.push_back(0.0f);
    circleVertices.push_back(0.5f);
    circleVertices.push_back(0.5f);

    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / segments * 2.0f * 3.14159f;
        circleVertices.push_back(cos(angle) * 0.5f);
        circleVertices.push_back(sin(angle) * 0.5f);
        circleVertices.push_back(0.0f);
        circleVertices.push_back(cos(angle) * 0.5f + 0.5f);
        circleVertices.push_back(sin(angle) * 0.5f + 0.5f);
    }

    for (int i = 1; i <= segments; i++) {
        circleIndices.push_back(0);
        circleIndices.push_back(i);
        circleIndices.push_back(i + 1);
    }

    assetManager->createMesh("circle", circleVertices.data(),
        circleVertices.size() * sizeof(float),
        circleIndices.data(), circleIndices.size());

    // Shader laden - WICHTIG: Pfad zur .shader Datei!
    // Bevorzugt: res/ECS_shaders (wie von dir genutzt). Fallback: res/shaders/ECS.shader
    Shader* loaded = assetManager->loadShader("default", "res/ECS_shaders");
    if (!loaded) {
        std::cout << "[RenderSystem] Primary shader path failed, trying res/shaders/ECS.shader...\n";
        loaded = assetManager->loadShader("default", "res/shaders/ECS.shader");
    } else {
        std::cout << "[RenderSystem] Shader loaded from res/ECS_shaders\n";
    }

    // Fallback: Erstelle einen einfachen Shader direkt im Code
    std::cout << "[RenderSystem] Creating fallback shader...\n";
    createFallbackShader();
}

void RenderSystem::update(EntityManager& em, float dt) {
    // ✅ FIX 1: Get entities with RenderComponent
    auto entities = em.getEntitiesWith<RenderComponent>();

    if (entities.empty()) {
        static int warnCounter = 0;
        if (++warnCounter % 120 == 0) {
            std::cout << "\n[RenderSystem] No entities with RenderComponent\n";
        }
        // don't return early; we still want to render ModelComponent entities
        //return;
    }

    // ✅ OPTIMIZATION: Only sort when needed instead of every frame
    if (needsResort || sortedEntities.size() != entities.size()) {
        sortedEntities = entities;
        std::sort(sortedEntities.begin(), sortedEntities.end(), [](Entity* a, Entity* b) {
            // Safety: Check if entities exist
            if (!a || !b) return false;
            if (!a->active || !b->active) return false;

            auto ra = a->getComponent<RenderComponent>();
            auto rb = b->getComponent<RenderComponent>();

            // Safety: Check if components exist
            if (!ra || !rb) return false;

            return ra->renderLayer < rb->renderLayer;
        });
        needsResort = false;
    }

    // Debug output (less frequent)
    static int frameCounter = 0;
    if (frameCounter++ % 120 == 0) {
        std::cout << "\r[RenderSystem] Entities: " << sortedEntities.size();
        for (size_t i = 0; i < sortedEntities.size() && i < 3; i++) {
            // ✅ Safety check before access
            if (!sortedEntities[i] || !sortedEntities[i]->active) continue;

            auto t = sortedEntities[i]->getComponent<TransformComponent>();
            auto r = sortedEntities[i]->getComponent<RenderComponent>();
            if (t && r) {
                std::cout << " | " << r->meshName << "(" << (int)t->position.x << "," << (int)t->position.y << ")";
            }
        }
        std::cout << std::flush;
    }

    // --------------------------
    // Render entities with RenderComponent, but skip if they have a ModelComponent
    // --------------------------
    for (Entity* e : sortedEntities) {
        // ✅ KRITISCH: Entity könnte während Iteration gelöscht worden sein!
        if (!e || !e->active) {
            continue;
        }

        // If entity has a ModelComponent, we prefer rendering via Model path
        if (e->hasComponent<ModelComponent>()) continue;

        auto transform = e->getComponent<TransformComponent>();
        auto render = e->getComponent<RenderComponent>();

        // ✅ KRITISCH: Components könnten nullptr sein!
        if (!render || !transform || !render->enabled) {
            continue;
        }

        // Assets abrufen
        Shader* shader = assetManager->getShader(render->shaderName);
        VertexArray* mesh = assetManager->getMesh(render->meshName);
        Texture* texture = render->textureName.empty() ? nullptr :
            assetManager->getTexture(render->textureName);

        // Shader handling
        if (!shader) {
            // Use fallback shader
            if (fallbackShaderID != 0) {
                glUseProgram(fallbackShaderID);
            }
            else {
                continue;
            }
        }
        else {
            shader->Bind();
        }

        // Mesh check
        if (!mesh) {
            continue;
        }

        // Model-View-Projection Matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(transform->position, 0.0f));
        model = glm::rotate(model, transform->rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(transform->scale, 1.0f));

        glm::mat4 mvp = projectionMatrix * viewMatrix * model;

        // Set uniforms
        if (shader) {
            shader->SetUniformMat4f("u_MVP", mvp);
            glm::vec4 colorWithAlpha(render->color, render->alpha);
            shader->SetUniform4f("u_color", colorWithAlpha.x, colorWithAlpha.y, colorWithAlpha.z, colorWithAlpha.w);
        }
        else {
            // ✅ OPTIMIZATION: Cache program ID to avoid repeated OpenGL queries
            if (cachedProgramID == 0) {
                GLint currentProgram;
                glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
                cachedProgramID = currentProgram;
            }
            int mvpLocation = glGetUniformLocation(cachedProgramID, "u_MVP");
            int colorLocation = glGetUniformLocation(cachedProgramID, "u_color");
            if (mvpLocation != -1) {
                glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
            }
            if (colorLocation != -1) {
                glm::vec4 colorWithAlpha(render->color, render->alpha);
                glUniform4f(colorLocation, colorWithAlpha.x, colorWithAlpha.y, colorWithAlpha.z, colorWithAlpha.w);
            }
        }

        // Texture handling
        if (texture) {
            glActiveTexture(GL_TEXTURE0);
            texture->Bind(0);
            if (shader) {
                shader->SetUniform1i("u_Texture", 0);
                shader->SetUniform1i("u_UseTexture", 1);
            }
            else {
                // ✅ OPTIMIZATION: Reuse cached program ID
                int texLoc = glGetUniformLocation(cachedProgramID, "u_Texture");
                int useTexLoc = glGetUniformLocation(cachedProgramID, "u_UseTexture");
                if (texLoc != -1) glUniform1i(texLoc, 0);
                if (useTexLoc != -1) glUniform1i(useTexLoc, 1);
            }
        }
        else {
            if (shader) {
                shader->SetUniform1i("u_UseTexture", 0);
            }
            else {
                // ✅ OPTIMIZATION: Reuse cached program ID
                int useTexLoc = glGetUniformLocation(cachedProgramID, "u_UseTexture");
                if (useTexLoc != -1) glUniform1i(useTexLoc, 0);
            }
        }

        // ✅ KRITISCH: Letzte Sicherheits-Checks vor Draw
        if (!mesh) continue;  // Nochmal prüfen

        try {
            mesh->Bind();
            glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
        }
        catch (...) {
            // Falls OpenGL error, nicht crashen
            std::cerr << "[RenderSystem] OpenGL error during draw\n";
        }
    }

    // --------------------------
    // Render entities with ModelComponent
    // --------------------------
    auto allEntities = em.getAllEntities();
    for (Entity* e : allEntities) {
        if (!e || !e->active) continue;
        if (!e->hasComponent<ModelComponent>()) continue;

        auto* t = e->getComponent<TransformComponent>();
        auto* m = e->getComponent<ModelComponent>();
        if (!m || !t) continue;

        // Use default shader if available, otherwise fallback program
        Shader* shader = assetManager->getShader("default");
        if (shader) shader->Bind(); else if (fallbackShaderID != 0) glUseProgram(fallbackShaderID); else continue;

        // For each shape in model, render using quad or circle mesh
        for (const auto& s : m->shapes) {
            VertexArray* mesh = nullptr;
            if (s.type == ModelComponent::ShapeType::Circle) mesh = assetManager->getMesh("circle");
            else if (s.type == ModelComponent::ShapeType::Triangle) mesh = assetManager->getMesh("triangle");
            else mesh = assetManager->getMesh("quad"); // rectangle fallback to quad
            if (!mesh) continue;

            // Build model matrix: entity transform * shape transform
            glm::mat4 model = glm::mat4(1.0f);
            // entity position
            model = glm::translate(model, glm::vec3(t->position, 0.0f));
            // shape local offset
            model = glm::translate(model, glm::vec3(s.position, 0.0f));
            // combined rotation: transform->rotation is in radians, s.rotation is degrees
            float totalRot = t->rotation + glm::radians(s.rotation);
            model = glm::rotate(model, totalRot, glm::vec3(0.0f, 0.0f, 1.0f));
            // scale: mesh is normalized to 1.0 -> apply shape size and scales
            glm::vec2 totalScale = s.size * s.scale; // size in pixels
            const float EPS = 1e-3f;
            if (std::abs(t->scale.x - 1.0f) < 10.0f && std::abs(t->scale.y - 1.0f) < 10.0f) {
                totalScale *= t->scale;
            }
            model = glm::scale(model, glm::vec3(totalScale, 1.0f));

            glm::mat4 mvp = projectionMatrix * viewMatrix * model;

            // Set uniforms for shader or fallback
            if (shader) {
                shader->SetUniformMat4f("u_MVP", mvp);
                glm::vec4 colorWithAlpha(s.color, 1.0f);
                shader->SetUniform4f("u_color", colorWithAlpha.x, colorWithAlpha.y, colorWithAlpha.z, colorWithAlpha.w);
            } else {
                int mvpLocation = glGetUniformLocation(cachedProgramID, "u_MVP");
                int colorLocation = glGetUniformLocation(cachedProgramID, "u_color");
                if (mvpLocation != -1) glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
                if (colorLocation != -1) glUniform4f(colorLocation, s.color.r, s.color.g, s.color.b, 1.0f);
            }

            // Texture handling if shape has texture
            if (!s.textureName.empty()) {
                Texture* tex = assetManager->getTexture(s.textureName);
                if (tex) {
                    glActiveTexture(GL_TEXTURE0);
                    tex->Bind(0);
                    if (shader) {
                        shader->SetUniform1i("u_Texture", 0);
                        shader->SetUniform1i("u_UseTexture", 1);
                    } else {
                        int texLoc = glGetUniformLocation(cachedProgramID, "u_Texture");
                        int useTexLoc = glGetUniformLocation(cachedProgramID, "u_UseTexture");
                        if (texLoc != -1) glUniform1i(texLoc, 0);
                        if (useTexLoc != -1) glUniform1i(useTexLoc, 1);
                    }
                } else {
                    if (shader) shader->SetUniform1i("u_UseTexture", 0);
                    else {
                        int useTexLoc = glGetUniformLocation(cachedProgramID, "u_UseTexture");
                        if (useTexLoc != -1) glUniform1i(useTexLoc, 0);
                    }
                }
            } else {
                if (shader) shader->SetUniform1i("u_UseTexture", 0);
                else {
                    int useTexLoc = glGetUniformLocation(cachedProgramID, "u_UseTexture");
                    if (useTexLoc != -1) glUniform1i(useTexLoc, 0);
                }
            }

            // Draw
            try {
                mesh->Bind();
                glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
            } catch (...) {
                std::cerr << "[RenderSystem] OpenGL error during model draw\n";
            }
        }
    }
}
void RenderSystem::setViewMatrix(const glm::mat4& view) {
    viewMatrix = view;
}

void RenderSystem::setProjectionMatrix(const glm::mat4& proj) {
    projectionMatrix = proj;
}

void RenderSystem::createFallbackShader() {
    // Einfacher Vertex Shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 a_Position;
        layout (location = 1) in vec2 a_TexCoord;
        uniform mat4 u_MVP;
        out vec2 v_TexCoord;
        void main() {
            gl_Position = u_MVP * vec4(a_Position, 1.0);
            v_TexCoord = a_TexCoord;
        }
    )";

    // Einfacher Fragment Shader mit Texture support
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 color;
        in vec2 v_TexCoord;
        uniform vec4 u_color;
        uniform sampler2D u_Texture;
        uniform int u_UseTexture;
        void main() {
            if (u_UseTexture == 1) {
                vec4 texColor = texture(u_Texture, v_TexCoord);
                color = texColor * u_color;
            } else {
                color = u_color;
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
        std::cout << "[RenderSystem] Fallback shader created with ID: " << shaderProgram << "\n";
        fallbackShaderID = shaderProgram;
        cachedProgramID = shaderProgram; // Cache the program ID
    } catch (const std::exception& e) {
        std::cout << "[RenderSystem] Failed to store fallback shader: " << e.what() << "\n";
    }
}
