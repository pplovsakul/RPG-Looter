#include "AssetManagerWindow.h"
#include "Texture.h"
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
        if (ImGui::BeginTabItem("Textures")) {
            drawTextureSection(em);
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Sounds")) {
            drawSoundSection(em);
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Models")) {
            drawModelSection(em);
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

void AssetManagerWindow::drawTextureSection(EntityManager& em) {
    auto texNames = AssetManager::getInstance()->getTextureNames();
    
    ImGui::Text("Textures (%zu loaded)", texNames.size());
    ImGui::Separator();
    
    // Search bar
    ImGui::InputTextWithHint("##texSearch", "Search textures...", textureSearchBuffer, sizeof(textureSearchBuffer));
    
    // Preview size slider
    ImGui::SliderInt("Preview Size", &texturePreviewSize, 32, 128);
    
    ImGui::Separator();
    
    // Texture list with previews
    ImGui::BeginChild("TexturesList", ImVec2(0, 300), true);
    
    for (const auto& name : texNames) {
        // Search filter
        if (textureSearchBuffer[0] != '\0') {
            std::string nameLower = name;
            std::string searchLower = textureSearchBuffer;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
            if (nameLower.find(searchLower) == std::string::npos) continue;
        }
        
        Texture* t = AssetManager::getInstance()->getTexture(name);
        
        if (t) {
            ImGui::Image((void*)(intptr_t)t->GetRendererID(), 
                        ImVec2(texturePreviewSize, texturePreviewSize));
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("%s", name.c_str());
                ImGui::Text("Size: %d x %d", t->GetWidth(), t->GetHeight());
                ImGui::Image((void*)(intptr_t)t->GetRendererID(), ImVec2(128, 128));
                ImGui::EndTooltip();
            }
        } else {
            ImGui::Dummy(ImVec2(texturePreviewSize, texturePreviewSize));
        }
        
        ImGui::SameLine();
        ImGui::Text("%s", name.c_str());
    }
    
    ImGui::EndChild();
    
    ImGui::Separator();
    ImGui::Text("Load New Texture:");
    ImGui::InputText("Name##tex", newTextureName, sizeof(newTextureName));
    ImGui::InputText("Path##tex", newTexturePath, sizeof(newTexturePath));
    if (ImGui::Button("Load Texture", ImVec2(150, 0))) {
        std::string name(newTextureName);
        std::string path(newTexturePath);
        if (AssetManager::getInstance()->loadTexture(name, path)) {
            ImGui::SameLine(); 
            ImGui::TextColored(ImVec4(0,1,0,1), "Loaded!");
        } else {
            ImGui::SameLine(); 
            ImGui::TextColored(ImVec4(1,0,0,1), "Failed");
        }
    }
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
    
    ImGui::Text("Models (%zu loaded)", modelNames.size());
    ImGui::Separator();
    
    // Search bar
    ImGui::InputTextWithHint("##modelSearch", "Search models...", modelSearchBuffer, sizeof(modelSearchBuffer));
    ImGui::Separator();
    
    // Separate into textured and non-textured models
    std::vector<std::string> texturedModels;
    std::vector<std::string> plainModels;
    
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
            plainModels.push_back(name); 
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
        
        if (hasTexture) texturedModels.push_back(name);
        else plainModels.push_back(name);
    }
    
    // Display in columns
    if (ImGui::BeginTable("ModelsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Textured Models");
        ImGui::TableSetupColumn("Plain Models");
        ImGui::TableHeadersRow();
        
        size_t maxRows = std::max(texturedModels.size(), plainModels.size());
        for (size_t i = 0; i < maxRows; i++) {
            ImGui::TableNextRow();
            
            // Textured models column
            ImGui::TableNextColumn();
            if (i < texturedModels.size()) {
                const auto& name = texturedModels[i];
                ImGui::Text("[Tex] %s", name.c_str());
                
                ModelComponent* m = AssetManager::getInstance()->getModel(name);
                if (m) {
                    ImGui::Indent();
                    ImGui::Text("Meshes: %zu", m->meshes.size());
                    ImGui::Unindent();
                }
            }
            
            // Plain models column
            ImGui::TableNextColumn();
            if (i < plainModels.size()) {
                const auto& name = plainModels[i];
                ImGui::Text("%s", name.c_str());
                
                ModelComponent* m = AssetManager::getInstance()->getModel(name);
                if (m) {
                    ImGui::Indent();
                    ImGui::Text("Meshes: %zu", m->meshes.size());
                    ImGui::Unindent();
                }
            }
        }
        
        ImGui::EndTable();
    }
}

void AssetManagerWindow::drawAssetStatistics() {
    ImGui::Text("Asset Statistics");
    ImGui::Separator();
    
    auto texNames = AssetManager::getInstance()->getTextureNames();
    auto sndNames = AssetManager::getInstance()->getSoundNames();
    auto modelNames = AssetManager::getInstance()->getModelNames();
    
    ImGui::Text("Total Assets: %zu", texNames.size() + sndNames.size() + modelNames.size());
    ImGui::Spacing();
    
    // Asset counts
    ImGui::BulletText("Textures: %zu", texNames.size());
    ImGui::BulletText("Sounds: %zu", sndNames.size());
    ImGui::BulletText("Models: %zu", modelNames.size());
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Texture details
    if (ImGui::CollapsingHeader("Texture Details")) {
        size_t totalPixels = 0;
        for (const auto& name : texNames) {
            Texture* t = AssetManager::getInstance()->getTexture(name);
            if (t) {
                int w = t->GetWidth();
                int h = t->GetHeight();
                totalPixels += w * h;
                ImGui::Text("%s: %dx%d", name.c_str(), w, h);
            }
        }
        ImGui::Separator();
        ImGui::Text("Total Pixels: %zu", totalPixels);
        ImGui::Text("Approx Memory: ~%.2f MB", (totalPixels * 4) / (1024.0f * 1024.0f));
    }
    
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
    ImGui::Text("CPU Stress Test (Simple Colored Quads):");
    ImGui::InputInt("CPU spawn count", &cpuSpawnCount);
    if (cpuSpawnCount < 0) cpuSpawnCount = 0;
    if (ImGui::Button("Spawn CPU Entities", ImVec2(180, 0))) {
        // Spawn many simple entities (no textures) to stress CPU
        for (int i = 0; i < cpuSpawnCount; ++i) {
            Entity* ent = em.createEntity();
            ent->tag = "CPUTest";
            ent->addComponent<TransformComponent>();
            auto tc = ent->getComponent<TransformComponent>();
            tc->position = glm::vec3(rand() % 1920, rand() % 1080, 0.0f);
            tc->scale = glm::vec3(10.0f, 10.0f, 1.0f);
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
    ImGui::Text("GPU Stress Test (Textured Quads):");
    ImGui::InputInt("GPU spawn count", &gpuSpawnCount);
    if (gpuSpawnCount < 0) gpuSpawnCount = 0;
    if (ImGui::Button("Spawn GPU Entities", ImVec2(180, 0))) {
        // Spawn many textured quads to stress GPU
        // Choose a texture if available
        std::string texToUse;
        auto tnames = AssetManager::getInstance()->getTextureNames();
        if (!tnames.empty()) texToUse = tnames[0];

        for (int i = 0; i < gpuSpawnCount; ++i) {
            Entity* ent = em.createEntity();
            ent->tag = "GPUTest";
            ent->addComponent<TransformComponent>();
            auto tc = ent->getComponent<TransformComponent>();
            tc->position = glm::vec3(rand() % 1920, rand() % 1080, 0.0f);
            tc->scale = glm::vec3(64.0f, 64.0f, 1.0f);
            ent->addComponent<RenderComponent>();
            auto rc = ent->getComponent<RenderComponent>();
            rc->meshName = "quad";
            rc->shaderName = "default";
            rc->textureName = texToUse;
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