#include "AssetManagerWindow.h"
#include "Components.h"
#include <sstream>

void AssetManagerWindow::update(EntityManager& em, float /*deltaTime*/) {
    auto& settings = GlobalSettings::getInstance();
    
    // Only show if the window is visible
    if (!settings.windowVisibility.showAssetManager) {
        return;
    }
    
    ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Asset Manager", &settings.windowVisibility.showAssetManager)) {
        ImGui::End();
        return;
    }
    
    // Tabs for better organization
    if (ImGui::BeginTabBar("AssetTabs")) {
        if (ImGui::BeginTabItem("Models")) {
            drawModelSection(em);
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Sounds")) {
            drawSoundSection(em);
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Statistics")) {
            drawAssetStatistics();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Stress Test")) {
            drawStressTestSection(em);
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void AssetManagerWindow::drawSoundSection(EntityManager& em) {
    auto sndNames = AssetManager::getInstance()->getSoundNames();
    
    ImGui::Text("Sounds (%zu loaded)", sndNames.size());
    ImGui::Separator();
    
    // Search bar
    ImGui::InputTextWithHint("##sndSearch", "Search sounds...", soundSearchBuffer, sizeof(soundSearchBuffer));
    ImGui::Separator();
    
    // Sound list
    ImGui::BeginChild("SoundsList", ImVec2(0, 300), true);
    
    for (const auto& name : sndNames) {
        // Search filter
        if (soundSearchBuffer[0] != '\0') {
            std::string nameLower = name;
            std::string searchLower = soundSearchBuffer;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
            if (nameLower.find(searchLower) == std::string::npos) continue;
        }
        
        ImGui::Text("%s", name.c_str());
        ImGui::SameLine();
        
        if (ImGui::SmallButton((std::string("Play##") + name).c_str())) {
            // Placeholder for sound playback
        }
        
        ImGui::SameLine();
        if (ImGui::SmallButton((std::string("Stop##") + name).c_str())) {
            // Placeholder for stopping sound
        }
    }
    
    ImGui::EndChild();
    
    ImGui::Separator();
    ImGui::Text("Load New Sound:");
    ImGui::InputText("Name##snd", newSoundName, sizeof(newSoundName));
    ImGui::InputText("Path##snd", newSoundPath, sizeof(newSoundPath));
    if (ImGui::Button("Load Sound", ImVec2(150, 0))) {
        std::string name(newSoundName);
        std::string path(newSoundPath);
        if (AssetManager::getInstance()->loadSound(name, path)) {
            ImGui::SameLine(); 
            ImGui::TextColored(ImVec4(0,1,0,1), "Loaded!");
        } else {
            ImGui::SameLine(); 
            ImGui::TextColored(ImVec4(1,0,0,1), "Failed");
        }
    }
}

void AssetManagerWindow::drawModelSection(EntityManager& em) {
    auto modelNames = AssetManager::getInstance()->getModelNames();
    
    ImGui::Text("3D Models (OBJ/MTL) - %zu loaded", modelNames.size());
    ImGui::Separator();
    
    // Search bar
    ImGui::InputTextWithHint("##modelSearch", "Search models...", modelSearchBuffer, sizeof(modelSearchBuffer));
    ImGui::Separator();
    
    // Model list
    ImGui::BeginChild("ModelsList", ImVec2(0, 300), true);
    
    for (const auto& name : modelNames) {
        // Search filter
        if (modelSearchBuffer[0] != '\0') {
            std::string nameLower = name;
            std::string searchLower = modelSearchBuffer;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
            if (nameLower.find(searchLower) == std::string::npos) continue;
        }
        
        ModelComponent* m = AssetManager::getInstance()->getModel(name);
        if (!m) { 
            ImGui::Text("%s (invalid)", name.c_str()); 
            continue; 
        }
        
        // Check if any mesh has a texture
        bool hasTexture = false;
        for (const auto& mesh : m->meshes) {
            if (!mesh.textureName.empty()) { 
                hasTexture = true; 
                break; 
            }
        }
        
        if (hasTexture) {
            ImGui::Text("[Textured] %s", name.c_str());
        } else {
            ImGui::Text("[Plain] %s", name.c_str());
        }
        ImGui::Indent();
        ImGui::Text("Meshes: %zu", m->meshes.size());
        size_t totalVerts = 0;
        for (const auto& mesh : m->meshes) {
            totalVerts += mesh.vertices.size();
        }
        ImGui::Text("Vertices: %zu", totalVerts);
        ImGui::Unindent();
    }
    
    ImGui::EndChild();
    
    ImGui::Separator();
    ImGui::Text("Load New 3D Model (OBJ/MTL):");
    ImGui::InputText("Name##model", newModelName, sizeof(newModelName));
    ImGui::InputText("Path##model", newModelPath, sizeof(newModelPath));
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
        "Tip: Path should point to .obj file. MTL will be loaded automatically.");
    
    if (ImGui::Button("Load Model", ImVec2(150, 0))) {
        std::string name(newModelName);
        std::string path(newModelPath);
        if (AssetManager::getInstance()->loadModelFromFile(name, path)) {
            ImGui::SameLine(); 
            ImGui::TextColored(ImVec4(0,1,0,1), "Loaded!");
        } else {
            ImGui::SameLine(); 
            ImGui::TextColored(ImVec4(1,0,0,1), "Failed");
        }
    }
}

void AssetManagerWindow::drawAssetStatistics() {
    ImGui::Text("Asset Statistics");
    ImGui::Separator();
    
    auto sndNames = AssetManager::getInstance()->getSoundNames();
    auto modelNames = AssetManager::getInstance()->getModelNames();
    
    ImGui::Text("Total Assets: %zu", sndNames.size() + modelNames.size());
    ImGui::Spacing();
    
    // Asset counts
    ImGui::BulletText("Sounds: %zu", sndNames.size());
    ImGui::BulletText("Models: %zu", modelNames.size());
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Model details
    if (ImGui::CollapsingHeader("Model Details")) {
        size_t totalMeshes = 0;
        size_t totalVertices = 0;
        for (const auto& name : modelNames) {
            ModelComponent* m = AssetManager::getInstance()->getModel(name);
            if (m) {
                totalMeshes += m->meshes.size();
                size_t verts = 0;
                for (const auto& mesh : m->meshes) {
                    verts += mesh.vertices.size();
                }
                totalVertices += verts;
                ImGui::Text("%s: %zu meshes, %zu vertices", name.c_str(), m->meshes.size(), verts);
            }
        }
        ImGui::Separator();
        ImGui::Text("Total Meshes: %zu", totalMeshes);
        ImGui::Text("Total Vertices: %zu", totalVertices);
    }
}

void AssetManagerWindow::drawStressTestSection(EntityManager& em) {
    ImGui::Text("Performance Stress Tests");
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Warning: High values may cause lag!");
    
    ImGui::Spacing();
    
    // CPU test
    ImGui::Text("CPU Stress Test (Simple 3D Objects):");
    ImGui::InputInt("CPU spawn count", &cpuSpawnCount);
    if (cpuSpawnCount < 0) cpuSpawnCount = 0;
    if (ImGui::Button("Spawn CPU Entities", ImVec2(180, 0))) {
        // Spawn many simple entities (no textures) to stress CPU
        for (int i = 0; i < cpuSpawnCount; ++i) {
            Entity* ent = em.createEntity();
            ent->tag = "CPUTest";
            ent->addComponent<TransformComponent>();
            auto tc = ent->getComponent<TransformComponent>();
            // Use 3D world coordinates instead of 2D screen space
            tc->position = glm::vec3(
                (rand() % 100) - 50.0f,  // X: -50 to 50
                (rand() % 100) - 50.0f,  // Y: -50 to 50
                (rand() % 100) - 50.0f   // Z: -50 to 50
            );
            tc->scale = glm::vec3(1.0f, 1.0f, 1.0f);
            ent->addComponent<RenderComponent>();
            auto rc = ent->getComponent<RenderComponent>();
            rc->meshName = "quad";
            rc->shaderName = "default";
            rc->color = glm::vec3(1.0f, 0.0f, 0.0f);
            spawnedCPUEntities.push_back(ent->id);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear CPU Entities", ImVec2(180, 0))) {
        for (auto id : spawnedCPUEntities) em.destroyEntity(id);
        spawnedCPUEntities.clear();
    }
    ImGui::Text("Current CPU test entities: %zu", spawnedCPUEntities.size());

    ImGui::Spacing();
    ImGui::Separator();
    
    // GPU test
    ImGui::Text("GPU Stress Test (3D Model Objects):");
    ImGui::InputInt("GPU spawn count", &gpuSpawnCount);
    if (gpuSpawnCount < 0) gpuSpawnCount = 0;
    if (ImGui::Button("Spawn GPU Entities", ImVec2(180, 0))) {
        // Spawn many 3D model objects to stress GPU
        // Choose a model if available
        std::string modelToUse;
        auto modelNames = AssetManager::getInstance()->getModelNames();
        if (!modelNames.empty()) modelToUse = modelNames[0];

        for (int i = 0; i < gpuSpawnCount; ++i) {
            Entity* ent = em.createEntity();
            ent->tag = "GPUTest";
            ent->addComponent<TransformComponent>();
            auto tc = ent->getComponent<TransformComponent>();
            // Use 3D world coordinates instead of 2D screen space
            tc->position = glm::vec3(
                (rand() % 100) - 50.0f,  // X: -50 to 50
                (rand() % 100) - 50.0f,  // Y: -50 to 50
                (rand() % 100) - 50.0f   // Z: -50 to 50
            );
            tc->scale = glm::vec3(2.0f, 2.0f, 2.0f);
            
            // If we have a model, add ModelComponent
            if (!modelToUse.empty()) {
                auto* modelSrc = AssetManager::getInstance()->getModel(modelToUse);
                if (modelSrc) {
                    auto* mc = ent->addComponent<ModelComponent>();
                    mc->meshes = modelSrc->meshes;
                }
            } else {
                // Fallback to RenderComponent if no models available
                ent->addComponent<RenderComponent>();
                auto rc = ent->getComponent<RenderComponent>();
                rc->meshName = "cube";
                rc->shaderName = "default";
                rc->color = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            spawnedGPUEntities.push_back(ent->id);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear GPU Entities", ImVec2(180, 0))) {
        for (auto id : spawnedGPUEntities) em.destroyEntity(id);
        spawnedGPUEntities.clear();
    }
    ImGui::Text("Current GPU test entities: %zu", spawnedGPUEntities.size());
    
    ImGui::Spacing();
    ImGui::Separator();
    
    if (ImGui::Button("Clear All Test Entities", ImVec2(200, 0))) {
        for (auto id : spawnedCPUEntities) em.destroyEntity(id);
        for (auto id : spawnedGPUEntities) em.destroyEntity(id);
        spawnedCPUEntities.clear();
        spawnedGPUEntities.clear();
    }
}