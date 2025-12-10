#include "DebugWindow.h"
#include "Components.h"
#include "vendor/imgui/imgui.h"
#include <cstring>

DebugWindow::DebugWindow(EntityManager* em, CameraController* camera, RenderSystem3D* renderer)
    : entityManager(em), cameraController(camera), renderSystem(renderer),
      showPerformance(true), showCameraInfo(true), showEntityList(true),
      showRenderStats(true), showLightingControls(true),
      fpsHistoryIndex(0), avgFPS(0.0f), lastFrameTime(0.0f),
      selectedEntity(nullptr) {
    
    std::memset(fpsHistory, 0, sizeof(fpsHistory));
}

void DebugWindow::render(float deltaTime) {
    lastFrameTime = deltaTime;
    float fps = deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;
    updateFPSHistory(fps);
    
    if (showPerformance) renderPerformanceWindow();
    if (showCameraInfo) renderCameraWindow();
    if (showEntityList) renderEntityListWindow();
    if (showRenderStats) renderRenderStatsWindow();
    if (showLightingControls) renderLightingControlsWindow();
    
    if (selectedEntity) {
        renderEntityInspector();
    }
}

void DebugWindow::renderPerformanceWindow() {
    ImGui::Begin("Performance", &showPerformance);
    
    ImGui::Text("FPS: %.1f", avgFPS);
    ImGui::Text("Frame Time: %.3f ms", lastFrameTime * 1000.0f);
    
    ImGui::PlotLines("FPS History", fpsHistory, 100, fpsHistoryIndex, nullptr, 0.0f, 120.0f, ImVec2(0, 80));
    
    ImGui::End();
}

void DebugWindow::renderCameraWindow() {
    if (!cameraController) return;
    
    ImGui::Begin("Camera Info", &showCameraInfo);
    
    Camera* camera = cameraController->getActiveCamera();
    if (camera) {
        glm::vec3 pos = camera->getPosition();
        glm::vec3 front = camera->getFront();
        float yaw = camera->getYaw();
        float pitch = camera->getPitch();
        float zoom = camera->getZoom();
        
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
        ImGui::Text("Front: (%.2f, %.2f, %.2f)", front.x, front.y, front.z);
        ImGui::Text("Yaw: %.2f°", yaw);
        ImGui::Text("Pitch: %.2f°", pitch);
        ImGui::Text("FOV/Zoom: %.1f°", zoom);
        
        ImGui::Separator();
        
        if (ImGui::Button("Reset Camera")) {
            // Reset camera position
        }
        
        bool mouseCapture = cameraController->isMouseCaptured();
        if (ImGui::Checkbox("Mouse Captured", &mouseCapture)) {
            cameraController->captureMouse(mouseCapture);
        }
    } else {
        ImGui::Text("No active camera");
    }
    
    ImGui::End();
}

void DebugWindow::renderEntityListWindow() {
    if (!entityManager) return;
    
    ImGui::Begin("Entity List", &showEntityList);
    
    auto entities = entityManager->getAllEntities();
    ImGui::Text("Total Entities: %zu", entities.size());
    
    ImGui::Separator();
    
    for (auto* entity : entities) {
        std::string label = "Entity " + std::to_string(entity->id);
        if (!entity->tag.empty()) {
            label += " (" + entity->tag + ")";
        }
        
        if (ImGui::Selectable(label.c_str(), selectedEntity == entity)) {
            selectedEntity = entity;
        }
    }
    
    ImGui::End();
}

void DebugWindow::renderRenderStatsWindow() {
    ImGui::Begin("Render Statistics", &showRenderStats);
    
    if (entityManager) {
        int meshCount = 0;
        int visibleCount = 0;
        
        auto entities = entityManager->getAllEntities();
        for (auto* entity : entities) {
            if (entity->getComponent<MeshComponent>()) {
                meshCount++;
                auto* meshComp = entity->getComponent<MeshComponent>();
                if (meshComp->visible) {
                    visibleCount++;
                }
            }
        }
        
        ImGui::Text("Total Meshes: %d", meshCount);
        ImGui::Text("Visible Meshes: %d", visibleCount);
    }
    
    ImGui::End();
}

void DebugWindow::renderLightingControlsWindow() {
    if (!renderSystem) return;
    
    ImGui::Begin("Lighting Controls", &showLightingControls);
    
    glm::vec3 lightDir = renderSystem->getLightDirection();
    glm::vec3 lightColor = renderSystem->getLightColor();
    glm::vec3 ambientColor = renderSystem->getAmbientColor();
    
    float lightDirArr[3] = { lightDir.x, lightDir.y, lightDir.z };
    float lightColorArr[3] = { lightColor.x, lightColor.y, lightColor.z };
    float ambientColorArr[3] = { ambientColor.x, ambientColor.y, ambientColor.z };
    
    ImGui::Text("Directional Light");
    if (ImGui::SliderFloat3("Direction", lightDirArr, -1.0f, 1.0f)) {
        renderSystem->setDirectionalLight(glm::vec3(lightDirArr[0], lightDirArr[1], lightDirArr[2]), lightColor);
    }
    
    if (ImGui::ColorEdit3("Light Color", lightColorArr)) {
        renderSystem->setDirectionalLight(lightDir, glm::vec3(lightColorArr[0], lightColorArr[1], lightColorArr[2]));
    }
    
    ImGui::Separator();
    
    ImGui::Text("Ambient Light");
    if (ImGui::ColorEdit3("Ambient Color", ambientColorArr)) {
        renderSystem->setAmbientLight(glm::vec3(ambientColorArr[0], ambientColorArr[1], ambientColorArr[2]));
    }
    
    ImGui::End();
}

void DebugWindow::renderEntityInspector() {
    if (!selectedEntity) return;
    
    ImGui::Begin("Entity Inspector", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    
    ImGui::Text("Entity ID: %d", selectedEntity->id);
    ImGui::InputText("Tag", &selectedEntity->tag[0], 64);
    
    ImGui::Separator();
    
    // Transform Component
    auto* transform = selectedEntity->getComponent<TransformComponent>();
    if (transform) {
        ImGui::Text("Transform Component");
        // Create local copies to avoid issues with glm::vec3 member access
        float pos[3] = { transform->position.x, transform->position.y, transform->position.z };
        float rot[3] = { transform->rotation.x, transform->rotation.y, transform->rotation.z };
        float scl[3] = { transform->scale.x, transform->scale.y, transform->scale.z };
        
        if (ImGui::DragFloat3("Position", pos, 0.1f)) {
            transform->position = glm::vec3(pos[0], pos[1], pos[2]);
        }
        if (ImGui::DragFloat3("Rotation", rot, 0.01f)) {
            transform->rotation = glm::vec3(rot[0], rot[1], rot[2]);
        }
        if (ImGui::DragFloat3("Scale", scl, 0.1f)) {
            transform->scale = glm::vec3(scl[0], scl[1], scl[2]);
        }
        ImGui::Separator();
    }
    
    // Mesh Component
    auto* meshComp = selectedEntity->getComponent<MeshComponent>();
    if (meshComp) {
        ImGui::Text("Mesh Component");
        ImGui::Text("Mesh: %s", meshComp->meshName.c_str());
        
        float diff[3] = { meshComp->diffuseColor.x, meshComp->diffuseColor.y, meshComp->diffuseColor.z };
        float spec[3] = { meshComp->specularColor.x, meshComp->specularColor.y, meshComp->specularColor.z };
        
        if (ImGui::ColorEdit3("Diffuse Color", diff)) {
            meshComp->diffuseColor = glm::vec3(diff[0], diff[1], diff[2]);
        }
        if (ImGui::ColorEdit3("Specular Color", spec)) {
            meshComp->specularColor = glm::vec3(spec[0], spec[1], spec[2]);
        }
        ImGui::SliderFloat("Shininess", &meshComp->shininess, 1.0f, 256.0f);
        ImGui::Checkbox("Visible", &meshComp->visible);
        ImGui::Checkbox("Wireframe", &meshComp->wireframe);
        ImGui::Separator();
    }
    
    // Character Controller Component
    auto* controller = selectedEntity->getComponent<CharacterControllerComponent>();
    if (controller) {
        ImGui::Text("Character Controller");
        ImGui::DragFloat("Move Speed", &controller->moveSpeed, 0.1f);
        ImGui::DragFloat("Jump Force", &controller->jumpForce, 0.1f);
        ImGui::Checkbox("Use Gravity", &controller->useGravity);
        ImGui::Text("Is Grounded: %s", controller->isGrounded ? "Yes" : "No");
        ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", 
                   controller->velocity.x, controller->velocity.y, controller->velocity.z);
    }
    
    ImGui::End();
}

void DebugWindow::updateFPSHistory(float fps) {
    fpsHistory[fpsHistoryIndex] = fps;
    fpsHistoryIndex = (fpsHistoryIndex + 1) % 100;
    
    // Calculate average
    float sum = 0.0f;
    for (int i = 0; i < 100; i++) {
        sum += fpsHistory[i];
    }
    avgFPS = sum / 100.0f;
}
