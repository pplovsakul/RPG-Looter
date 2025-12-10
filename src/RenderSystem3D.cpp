#include "RenderSystem3D.h"
#include "Components.h"
#include "Entitymanager.h"
#include "Shader.h"
#include <glad/glad.h>
#include <iostream>
#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"

RenderSystem3D::RenderSystem3D(AssetManager* assets, CameraController* camera)
    : assetManager(assets), cameraController(camera),
      defaultShaderID(0), lightingShaderID(0),
      lightDirection(-0.2f, -1.0f, -0.3f),
      lightColor(1.0f, 1.0f, 1.0f),
      ambientColor(0.3f, 0.3f, 0.3f) {
}

void RenderSystem3D::init() {
    createDefaultShaders();
}

void RenderSystem3D::update(EntityManager& em, float deltaTime) {
    if (!cameraController) return;
    
    Camera* camera = cameraController->getActiveCamera();
    if (!camera) return;
    
    // Enable depth testing for 3D
    glEnable(GL_DEPTH_TEST);
    
    // Get matrices from camera
    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 projection = camera->getProjectionMatrix();
    
    // Render all entities with MeshComponent
    auto entities = em.getAllEntities();
    for (auto* entity : entities) {
        auto* meshComp = entity->getComponent<MeshComponent>();
        auto* transform = entity->getComponent<TransformComponent>();
        
        if (meshComp && meshComp->visible && transform) {
            // Get mesh
            auto mesh = getMesh(meshComp->meshName);
            if (!mesh) continue;
            
            // Setup shader
            unsigned int shaderID = lightingShaderID;
            if (shaderID == 0) continue;
            
            glUseProgram(shaderID);
            
            // Calculate model matrix
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, transform->position);
            // Apply rotation for each axis (X, Y, Z)
            model = glm::rotate(model, transform->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, transform->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, transform->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, transform->scale);
            
            // Set matrices
            GLint modelLoc = glGetUniformLocation(shaderID, "uModel");
            GLint viewLoc = glGetUniformLocation(shaderID, "uView");
            GLint projLoc = glGetUniformLocation(shaderID, "uProjection");
            
            if (modelLoc != -1) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
            if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
            if (projLoc != -1) glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
            
            // Set material properties
            GLint diffuseLoc = glGetUniformLocation(shaderID, "uDiffuseColor");
            if (diffuseLoc != -1) {
                float diffuse[3] = { meshComp->diffuseColor.x, meshComp->diffuseColor.y, meshComp->diffuseColor.z };
                glUniform3fv(diffuseLoc, 1, diffuse);
            }
            
            // Set lighting
            setupLighting(shaderID);
            
            // Set camera position for specular
            GLint viewPosLoc = glGetUniformLocation(shaderID, "uViewPos");
            if (viewPosLoc != -1) {
                glm::vec3 camPos = camera->getPosition();
                float camPosArr[3] = { camPos.x, camPos.y, camPos.z };
                glUniform3fv(viewPosLoc, 1, camPosArr);
            }
            
            // Render mesh
            if (meshComp->wireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            
            mesh->draw();
            
            if (meshComp->wireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }
    }
    
    glUseProgram(0);
}

void RenderSystem3D::addMesh(const std::string& name, std::shared_ptr<Mesh3D> mesh) {
    meshes[name] = mesh;
    if (mesh) {
        mesh->setupBuffers();
    }
}

std::shared_ptr<Mesh3D> RenderSystem3D::getMesh(const std::string& name) {
    auto it = meshes.find(name);
    if (it != meshes.end()) {
        return it->second;
    }
    return nullptr;
}

void RenderSystem3D::setDirectionalLight(const glm::vec3& direction, const glm::vec3& color) {
    lightDirection = direction;
    lightColor = color;
}

void RenderSystem3D::setAmbientLight(const glm::vec3& color) {
    ambientColor = color;
}

void RenderSystem3D::createDefaultShaders() {
    // Create 3D lighting shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPosition;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoords;
        layout (location = 3) in vec3 aColor;
        
        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoords;
        out vec3 VertexColor;
        
        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;
        
        void main() {
            FragPos = vec3(uModel * vec4(aPosition, 1.0));
            Normal = mat3(transpose(inverse(uModel))) * aNormal;
            TexCoords = aTexCoords;
            VertexColor = aColor;
            
            gl_Position = uProjection * uView * vec4(FragPos, 1.0);
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoords;
        in vec3 VertexColor;
        
        uniform vec3 uLightDir;
        uniform vec3 uLightColor;
        uniform vec3 uAmbientColor;
        uniform vec3 uDiffuseColor;
        uniform vec3 uViewPos;
        
        void main() {
            // Ambient
            vec3 ambient = uAmbientColor;
            
            // Diffuse
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(-uLightDir);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * uLightColor;
            
            // Result
            vec3 result = (ambient + diffuse) * uDiffuseColor * VertexColor;
            FragColor = vec4(result, 1.0);
        }
    )";
    
    // Compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    // Check for compile errors
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }
    
    // Link program
    lightingShaderID = glCreateProgram();
    glAttachShader(lightingShaderID, vertexShader);
    glAttachShader(lightingShaderID, fragmentShader);
    glLinkProgram(lightingShaderID);
    
    glGetProgramiv(lightingShaderID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(lightingShaderID, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    defaultShaderID = lightingShaderID;
}

void RenderSystem3D::renderMesh(const MeshComponent* meshComp, const TransformComponent* transform) {
    // This method is not used in the current implementation
}

void RenderSystem3D::setupLighting(unsigned int shaderID) {
    GLint lightDirLoc = glGetUniformLocation(shaderID, "uLightDir");
    GLint lightColorLoc = glGetUniformLocation(shaderID, "uLightColor");
    GLint ambientColorLoc = glGetUniformLocation(shaderID, "uAmbientColor");
    
    float lightDirArr[3] = { lightDirection.x, lightDirection.y, lightDirection.z };
    float lightColorArr[3] = { lightColor.x, lightColor.y, lightColor.z };
    float ambientColorArr[3] = { ambientColor.x, ambientColor.y, ambientColor.z };
    
    if (lightDirLoc != -1) glUniform3fv(lightDirLoc, 1, lightDirArr);
    if (lightColorLoc != -1) glUniform3fv(lightColorLoc, 1, lightColorArr);
    if (ambientColorLoc != -1) glUniform3fv(ambientColorLoc, 1, ambientColorArr);
}
