#include "AssetManagerWindow.h"
#include "Texture.h"
#include "Components.h"
#include <sstream>

void AssetManagerWindow::update(EntityManager& em, float /*deltaTime*/) {
    ImGui::Begin("Asset Manager");

    // ---- Textures ----
    ImGui::Text("Textures:");
    auto texNames = AssetManager::getInstance()->getTextureNames();
    ImGui::BeginChild("TexturesList", ImVec2(300, 200), true);
    for (const auto& name : texNames) {
        ImGui::Text("%s", name.c_str());
        ImGui::SameLine();
        Texture* t = AssetManager::getInstance()->getTexture(name);
        if (t) {
            // small preview (scale down)
            ImGui::SameLine();
            ImGui::Image((void*)(intptr_t)t->GetRendererID(), ImVec2(48, 48));
        }
    }
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::Text("Load Texture:");
    ImGui::InputText("Name##tex", newTextureName, sizeof(newTextureName));
    ImGui::InputText("Path##tex", newTexturePath, sizeof(newTexturePath));
    if (ImGui::Button("Load Texture")) {
        std::string name(newTextureName);
        std::string path(newTexturePath);
        if (AssetManager::getInstance()->loadTexture(name, path)) {
            ImGui::TextColored(ImVec4(0,1,0,1), "Texture loaded");
        } else {
            ImGui::TextColored(ImVec4(1,0,0,1), "Failed to load texture");
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // ---- Sounds ----
    ImGui::Text("Sounds:");
    auto sndNames = AssetManager::getInstance()->getSoundNames();
    ImGui::BeginChild("SoundsList", ImVec2(300, 200), true);
    for (const auto& name : sndNames) {
        ImGui::Text("%s", name.c_str());
        ImGui::SameLine();
        // Optionally add a play button if you integrate with AudioSystem
        if (ImGui::SmallButton((std::string("Play##") + name).c_str())) {
            // If you want to auto-play sounds from the AssetManager,
            // integrate with AudioSystem; leaving as placeholder.
        }
    }
    ImGui::EndChild();

    ImGui::Separator();
    ImGui::Text("Load Sound:");
    ImGui::InputText("Name##snd", newSoundName, sizeof(newSoundName));
    ImGui::InputText("Path##snd", newSoundPath, sizeof(newSoundPath));
    if (ImGui::Button("Load Sound")) {
        std::string name(newSoundName);
        std::string path(newSoundPath);
        if (AssetManager::getInstance()->loadSound(name, path)) {
            ImGui::TextColored(ImVec4(0,1,0,1), "Sound loaded");
        } else {
            ImGui::TextColored(ImVec4(1,0,0,1), "Failed to load sound");
        }
    }

    ImGui::Spacing();
    ImGui::Separator();

    // ---- Models: split into Textured Models and Models ----
    auto modelNames = AssetManager::getInstance()->getModelNames();

    // Separate into textured and non-textured models
    std::vector<std::string> texturedModels;
    std::vector<std::string> plainModels;
    for (const auto& name : modelNames) {
        ModelComponent* m = AssetManager::getInstance()->getModel(name);
        if (!m) { plainModels.push_back(name); continue; }
        bool hasTexture = false;
        for (const auto& s : m->shapes) {
            if (!s.textureName.empty()) { hasTexture = true; break; }
        }
        if (hasTexture) texturedModels.push_back(name);
        else plainModels.push_back(name);
    }

    // Textured Models
    ImGui::Text("Textured Models:");
    ImGui::BeginChild("TexturedModelsList", ImVec2(300, 200), true);
    for (const auto& name : texturedModels) {
        ImGui::Text("%s", name.c_str());
        ImGui::SameLine();
        // try to show first texture found in model
        ModelComponent* m = AssetManager::getInstance()->getModel(name);
        if (m) {
            std::string foundTex;
            for (const auto& s : m->shapes) { if (!s.textureName.empty()) { foundTex = s.textureName; break; } }
            if (!foundTex.empty()) {
                Texture* t = AssetManager::getInstance()->getTexture(foundTex);
                if (t) {
                    ImGui::SameLine();
                    ImGui::Image((void*)(intptr_t)t->GetRendererID(), ImVec2(48,48));
                } else {
                    ImGui::SameLine(); ImGui::TextDisabled("(texture not loaded)");
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::SmallButton((std::string("PreviewModel##tex") + name).c_str())) {
            // Placeholder: preview textured model
        }
    }
    ImGui::EndChild();

    ImGui::Separator();

    // Plain Models
    ImGui::Text("Models:");
    ImGui::BeginChild("ModelsList", ImVec2(300, 200), true);
    for (const auto& name : plainModels) {
        ImGui::Text("%s", name.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton((std::string("PreviewModel##") + name).c_str())) {
            // Placeholder: preview model
        }
    }
    ImGui::EndChild();

    ImGui::Separator();

    // Performance Stress Test Controls
    ImGui::Text("Performance Stress Tests:");
    ImGui::InputInt("CPU spawn count", &cpuSpawnCount);
    if (cpuSpawnCount < 0) cpuSpawnCount = 0;
    if (ImGui::Button("Spawn CPU Entities")) {
        // Spawn many simple entities (no textures) to stress CPU
        for (int i = 0; i < cpuSpawnCount; ++i) {
            Entity* ent = em.createEntity();
            ent->addComponent<TransformComponent>();
            auto tc = ent->getComponent<TransformComponent>();
            tc->position = glm::vec2(rand() % 1920, rand() % 1080);
            tc->scale = glm::vec2(10.0f, 10.0f);
            ent->addComponent<RenderComponent>();
            auto rc = ent->getComponent<RenderComponent>();
            rc->meshName = "quad";
            rc->shaderName = "default";
            rc->color = glm::vec3(1.0f, 0.0f, 0.0f);
            spawnedCPUEntities.push_back(ent->id);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear CPU Entities")) {
        for (auto id : spawnedCPUEntities) em.destroyEntity(id);
        spawnedCPUEntities.clear();
    }

    ImGui::InputInt("GPU spawn count", &gpuSpawnCount);
    if (gpuSpawnCount < 0) gpuSpawnCount = 0;
    if (ImGui::Button("Spawn GPU Entities")) {
        // Spawn many textured quads to stress GPU
        // Choose a texture if available
        std::string texToUse;
        auto tnames = AssetManager::getInstance()->getTextureNames();
        if (!tnames.empty()) texToUse = tnames[0];

        for (int i = 0; i < gpuSpawnCount; ++i) {
            Entity* ent = em.createEntity();
            ent->addComponent<TransformComponent>();
            auto tc = ent->getComponent<TransformComponent>();
            tc->position = glm::vec2(rand() % 1920, rand() % 1080);
            tc->scale = glm::vec2(64.0f, 64.0f);
            ent->addComponent<RenderComponent>();
            auto rc = ent->getComponent<RenderComponent>();
            rc->meshName = "quad";
            rc->shaderName = "default";
            rc->textureName = texToUse;
            spawnedGPUEntities.push_back(ent->id);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear GPU Entities")) {
        for (auto id : spawnedGPUEntities) em.destroyEntity(id);
        spawnedGPUEntities.clear();
    }

    ImGui::End();
}