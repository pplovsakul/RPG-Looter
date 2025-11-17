#include "EditorSystem.h"
#include "EntitySerializer.h"   // <- neu
#include "AssetManager.h"
#include <cstring> // for strncpy

// temporary path buffers for the UI
static char savePathBuf[256] = "res/entities/entities.json";
static char loadPathBuf[256] = "res/entities/entities.json";

void EditorSystem::update(EntityManager& em, float /*deltaTime*/) {
    ImGui::SetNextWindowSize(ImVec2(500, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Entity Editor");

    // Tabs for better organization
    if (ImGui::BeginTabBar("EditorTabs")) {
        // Main editing tab
        if (ImGui::BeginTabItem("Edit")) {
            drawEntityEditingTab(em);
            ImGui::EndTabItem();
        }
        
        // Templates tab
        if (ImGui::BeginTabItem("Templates")) {
            drawComponentTemplates(em);
            ImGui::EndTabItem();
        }
        
        // Batch operations tab
        if (ImGui::BeginTabItem("Batch")) {
            drawBatchOperations(em);
            ImGui::EndTabItem();
        }
        
        // Save/Load tab
        if (ImGui::BeginTabItem("Save/Load")) {
            drawSaveLoadTab(em);
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
    
    ImGui::End();
}

void EditorSystem::drawEntityEditingTab(EntityManager& em) {
    // Search bar
    ImGui::InputTextWithHint("##search", "Search entities...", searchBuffer, sizeof(searchBuffer));
    ImGui::Separator();

    // Create entity UI with templates
    ImGui::Text("Create New Entity:");
    ImGui::InputText("Name", newEntityName, sizeof(newEntityName));
    if (ImGui::Button("Create Empty")) {
        Entity* e = em.createEntity();
        e->tag = std::string(newEntityName);
        selectedEntityId = e->id;
    }
    ImGui::SameLine();
    if (ImGui::Button("Create with Transform")) {
        Entity* e = em.createEntity();
        e->tag = std::string(newEntityName);
        e->addComponent<TransformComponent>();
        selectedEntityId = e->id;
    }
    ImGui::SameLine();
    if (ImGui::Button("Create Sprite")) {
        Entity* e = em.createEntity();
        e->tag = std::string(newEntityName);
        auto* t = e->addComponent<TransformComponent>();
        t->position = glm::vec2(960.0f, 540.0f);
        t->scale = glm::vec2(100.0f, 100.0f);
        auto* r = e->addComponent<RenderComponent>();
        r->meshName = "quad";
        r->shaderName = "default";
        selectedEntityId = e->id;
    }
    
    ImGui::Separator();

    // Entity list with search
    ImGui::Text("Entities (%zu):", em.getAllEntities().size());
    ImGui::BeginChild("EntityList", ImVec2(0, 250), true);
    
    for (auto* e : em.getAllEntities()) {
        // Search filter
        if (searchBuffer[0] != '\0') {
            std::string tag = e->tag;
            std::string search = searchBuffer;
            std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
            std::transform(search.begin(), search.end(), search.begin(), ::tolower);
            if (tag.find(search) == std::string::npos) continue;
        }
        
        char buf[128];
        // Add component indicators
        std::string indicators = "";
        if (e->hasComponent<TransformComponent>()) indicators += "[T]";
        if (e->hasComponent<RenderComponent>()) indicators += "[R]";
        if (e->hasComponent<AudioComponent>()) indicators += "[A]";
        if (e->hasComponent<ModelComponent>()) indicators += "[M]";
        
        snprintf(buf, sizeof(buf), "%s [%u] %s", indicators.c_str(), e->id, e->tag.c_str());
        bool sel = (selectedEntityId == (int)e->id);
        if (ImGui::Selectable(buf, sel)) {
            selectedEntityId = (int)e->id;
        }
        
        // Right-click context menu
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Duplicate")) {
                Entity* newE = em.createEntity();
                newE->tag = e->tag + "_copy";
                if (e->hasComponent<TransformComponent>()) {
                    auto* src = e->getComponent<TransformComponent>();
                    auto* dst = newE->addComponent<TransformComponent>();
                    *dst = *src;
                }
                if (e->hasComponent<RenderComponent>()) {
                    auto* src = e->getComponent<RenderComponent>();
                    auto* dst = newE->addComponent<RenderComponent>();
                    *dst = *src;
                }
            }
            if (ImGui::MenuItem("Copy Components")) {
                copyComponentsToClipboard(e);
            }
            if (ImGui::MenuItem("Delete")) {
                em.destroyEntity(e->id);
                if (selectedEntityId == (int)e->id) selectedEntityId = -1;
            }
            ImGui::EndPopup();
        }
    }
    ImGui::EndChild();

    // Quick actions
    if (selectedEntityId >= 0) {
        if (ImGui::Button("Destroy Selected")) {
            em.destroyEntity((unsigned int)selectedEntityId);
            selectedEntityId = -1;
        }
        ImGui::SameLine();
        Entity* selected = em.getEntityByID((unsigned int)selectedEntityId);
        if (selected) {
            if (ImGui::Button("Copy Components")) {
                copyComponentsToClipboard(selected);
            }
            ImGui::SameLine();
            if (ImGui::Button("Paste Components")) {
                pasteComponentsFromClipboard(selected);
            }
        }
    }

    ImGui::Separator();

    // Selected entity inspector
    Entity* selected = (selectedEntityId >= 0) ? em.getEntityByID((unsigned int)selectedEntityId) : nullptr;
    if (!selected) {
        ImGui::TextColored(ImVec4(1,0.5f,0,1), "No entity selected");
        return;
    }

    ImGui::Text("Selected: [%u] %s", selected->id, selected->tag.c_str());
    char tagBuf[128];
    strncpy(tagBuf, selected->tag.c_str(), sizeof(tagBuf)); tagBuf[sizeof(tagBuf)-1] = '\0';
    if (ImGui::InputText("Tag", tagBuf, sizeof(tagBuf))) {
        selected->tag = std::string(tagBuf);
    }

    // Component add buttons
    ImGui::Separator();
    ImGui::Text("Components:");
    if (!selected->hasComponent<TransformComponent>()) {
        if (ImGui::Button("Add Transform")) selected->addComponent<TransformComponent>();
    } else {
        if (ImGui::Button("Remove Transform")) selected->removeComponent<TransformComponent>();
    }
    ImGui::SameLine();
    if (!selected->hasComponent<RenderComponent>()) {
        if (ImGui::Button("Add Render")) selected->addComponent<RenderComponent>();
    } else {
        if (ImGui::Button("Remove Render")) selected->removeComponent<RenderComponent>();
    }
    ImGui::SameLine();
    if (!selected->hasComponent<AudioComponent>()) {
        if (ImGui::Button("Add Audio")) selected->addComponent<AudioComponent>();
    } else {
        if (ImGui::Button("Remove Audio")) selected->removeComponent<AudioComponent>();
    }
   

    ImGui::Separator();

    // Draw component editors
    if (selected->hasComponent<TransformComponent>()) drawTransformEditor(selected);
    if (selected->hasComponent<RenderComponent>()) drawRenderEditor(selected);
    if (selected->hasComponent<AudioComponent>()) drawAudioEditor(selected);

    // New: Model assignment UI
    drawModelEditor(selected);
}

void EditorSystem::drawSaveLoadTab(EntityManager& em) {
    ImGui::Text("Save/Load Entities");
    ImGui::Separator();
    
    ImGui::InputText("Save Path", savePathBuf, sizeof(savePathBuf));
    if (ImGui::Button("Save Entities", ImVec2(200, 0))) {
        bool ok = EntitySerializer::saveEntities(em, std::string(savePathBuf));
        if (ok) ImGui::SameLine(), ImGui::TextColored(ImVec4(0,1,0,1), "Saved!");
        else ImGui::SameLine(), ImGui::TextColored(ImVec4(1,0,0,1), "Save failed");
    }
    
    ImGui::Spacing();
    ImGui::InputText("Load Path", loadPathBuf, sizeof(loadPathBuf));
    if (ImGui::Button("Load Entities", ImVec2(200, 0))) {
        bool ok = EntitySerializer::loadEntities(em, std::string(loadPathBuf));
        if (ok) ImGui::SameLine(), ImGui::TextColored(ImVec4(0,1,0,1), "Loaded!");
        else ImGui::SameLine(), ImGui::TextColored(ImVec4(1,0,0,1), "Load failed");
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Tip: Use res/entities/ for entity files");
}

void EditorSystem::drawComponentTemplates(EntityManager& em) {
    ImGui::Text("Quick Entity Templates");
    ImGui::Separator();
    
    static char templateName[64] = "TemplateEntity";
    ImGui::InputText("Entity Name", templateName, sizeof(templateName));
    
    ImGui::Spacing();
    
    if (ImGui::Button("Empty Entity", ImVec2(200, 0))) {
        Entity* e = em.createEntity();
        e->tag = std::string(templateName);
        selectedEntityId = e->id;
    }
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No components");
    
    ImGui::Spacing();
    
    if (ImGui::Button("Sprite Entity", ImVec2(200, 0))) {
        Entity* e = em.createEntity();
        e->tag = std::string(templateName);
        auto* t = e->addComponent<TransformComponent>();
        t->position = glm::vec2(960.0f, 540.0f);
        t->scale = glm::vec2(100.0f, 100.0f);
        auto* r = e->addComponent<RenderComponent>();
        r->meshName = "quad";
        r->shaderName = "default";
        r->color = glm::vec3(1.0f, 1.0f, 1.0f);
        selectedEntityId = e->id;
    }
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Transform + Render");
    
    ImGui::Spacing();
    
    if (ImGui::Button("Audio Source", ImVec2(200, 0))) {
        Entity* e = em.createEntity();
        e->tag = std::string(templateName);
        e->addComponent<TransformComponent>();
        auto* a = e->addComponent<AudioComponent>();
        a->volume = 1.0f;
        a->pitch = 1.0f;
        selectedEntityId = e->id;
    }
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Transform + Audio");
    
    ImGui::Spacing();
    
    if (ImGui::Button("Complete Entity", ImVec2(200, 0))) {
        Entity* e = em.createEntity();
        e->tag = std::string(templateName);
        auto* t = e->addComponent<TransformComponent>();
        t->position = glm::vec2(960.0f, 540.0f);
        t->scale = glm::vec2(100.0f, 100.0f);
        auto* r = e->addComponent<RenderComponent>();
        r->meshName = "quad";
        r->shaderName = "default";
        auto* a = e->addComponent<AudioComponent>();
        a->volume = 1.0f;
        selectedEntityId = e->id;
    }
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Transform + Render + Audio");
}

void EditorSystem::drawBatchOperations(EntityManager& em) {
    ImGui::Text("Batch Operations");
    ImGui::Separator();
    
    static bool selectAll = false;
    if (ImGui::Checkbox("Select All Entities", &selectAll)) {
        // Placeholder - would need to track multiple selections
    }
    
    ImGui::Spacing();
    
    ImGui::Text("Delete Operations:");
    if (ImGui::Button("Delete All Entities", ImVec2(200, 0))) {
        ImGui::OpenPopup("ConfirmDeleteAll");
    }
    
    if (ImGui::BeginPopupModal("ConfirmDeleteAll", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete ALL entities?");
        ImGui::Text("This cannot be undone!");
        ImGui::Separator();
        
        if (ImGui::Button("Yes, Delete All", ImVec2(120, 0))) {
            auto entities = em.getAllEntities();
            for (auto* e : entities) {
                em.destroyEntity(e->id);
            }
            selectedEntityId = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    ImGui::Spacing();
    
    if (ImGui::Button("Delete Entities Without Transform", ImVec2(250, 0))) {
        auto entities = em.getAllEntities();
        for (auto* e : entities) {
            if (!e->hasComponent<TransformComponent>()) {
                em.destroyEntity(e->id);
            }
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    ImGui::Text("Component Operations:");
    if (ImGui::Button("Add Transform to All", ImVec2(200, 0))) {
        auto entities = em.getAllEntities();
        for (auto* e : entities) {
            if (!e->hasComponent<TransformComponent>()) {
                e->addComponent<TransformComponent>();
            }
        }
    }
}

void EditorSystem::copyComponentsToClipboard(Entity* e) {
    if (!e) return;
    
    clipboard.hasTransform = e->hasComponent<TransformComponent>();
    clipboard.hasRender = e->hasComponent<RenderComponent>();
    clipboard.hasAudio = e->hasComponent<AudioComponent>();
    
    if (clipboard.hasTransform) {
        clipboard.transform = *e->getComponent<TransformComponent>();
    }
    if (clipboard.hasRender) {
        clipboard.render = *e->getComponent<RenderComponent>();
    }
    if (clipboard.hasAudio) {
        clipboard.audio = *e->getComponent<AudioComponent>();
    }
}

void EditorSystem::pasteComponentsFromClipboard(Entity* e) {
    if (!e) return;
    
    if (clipboard.hasTransform) {
        if (!e->hasComponent<TransformComponent>()) {
            e->addComponent<TransformComponent>();
        }
        *e->getComponent<TransformComponent>() = clipboard.transform;
    }
    if (clipboard.hasRender) {
        if (!e->hasComponent<RenderComponent>()) {
            e->addComponent<RenderComponent>();
        }
        *e->getComponent<RenderComponent>() = clipboard.render;
    }
    if (clipboard.hasAudio) {
        if (!e->hasComponent<AudioComponent>()) {
            e->addComponent<AudioComponent>();
        }
        *e->getComponent<AudioComponent>() = clipboard.audio;
    }
}

void EditorSystem::drawTransformEditor(Entity* e) {
    auto* t = e->getComponent<TransformComponent>();
    if (!t) return;
    if (ImGui::CollapsingHeader("Transform")) {
        ImGui::DragFloat2("Position", &t->position.x, 1.0f);
        ImGui::DragFloat("Rotation", &t->rotation, 1.0f);
        ImGui::DragFloat2("Scale", &t->scale.x, 0.1f);
    }
}

void EditorSystem::drawRenderEditor(Entity* e) {
    auto* r = e->getComponent<RenderComponent>();
    if (!r) return;
    if (ImGui::CollapsingHeader("Render")) {
        char meshBuf[128]; strncpy(meshBuf, r->meshName.c_str(), sizeof(meshBuf)); meshBuf[sizeof(meshBuf)-1]=0;
        if (ImGui::InputText("Mesh", meshBuf, sizeof(meshBuf))) r->meshName = meshBuf;

        char shaderBuf[128]; strncpy(shaderBuf, r->shaderName.c_str(), sizeof(shaderBuf)); shaderBuf[sizeof(shaderBuf)-1]=0;
        if (ImGui::InputText("Shader", shaderBuf, sizeof(shaderBuf))) r->shaderName = shaderBuf;

        char texBuf[128]; strncpy(texBuf, r->textureName.c_str(), sizeof(texBuf)); texBuf[sizeof(texBuf)-1]=0;
        if (ImGui::InputText("Texture", texBuf, sizeof(texBuf))) r->textureName = texBuf;

        ImGui::ColorEdit3("Color", &r->color.x);
        ImGui::SliderFloat("Alpha", &r->alpha, 0.0f, 1.0f);
        ImGui::Checkbox("Enabled", &r->enabled);
        ImGui::InputInt("Layer", &r->renderLayer);
    }
}

void EditorSystem::drawAudioEditor(Entity* e) {
    auto* a = e->getComponent<AudioComponent>();
    if (!a) return;
    if (ImGui::CollapsingHeader("Audio")) {
        char buf[128]; strncpy(buf, a->soundName.c_str(), sizeof(buf)); buf[sizeof(buf)-1] = 0;
        if (ImGui::InputText("Sound", buf, sizeof(buf))) a->soundName = buf;
        ImGui::Checkbox("PlayOnce", &a->playOnce);
        ImGui::Checkbox("Loop", &a->loop);
        ImGui::SliderFloat("Volume", &a->volume, 0.0f, 1.0f);
        ImGui::SliderFloat("Pitch", &a->pitch, 0.1f, 3.0f);
    }
}

void EditorSystem::drawModelEditor(Entity* e) {
    if (!ImGui::CollapsingHeader("Model")) return;

    // If entity already has a ModelComponent, show its name and allow removal
    if (e->hasComponent<ModelComponent>()) {
        ImGui::Text("Entity has ModelComponent assigned");
        if (ImGui::Button("Remove ModelComponent")) {
            e->removeComponent<ModelComponent>();
        }
        ImGui::Separator();
    }

    // Assign model asset from AssetManager
    auto models = AssetManager::getInstance()->getModelNames();
    static int modelIdx = -1;
    std::string currentLabel = (modelIdx >=0 && modelIdx < (int)models.size()) ? models[modelIdx] : "None";
    if (ImGui::BeginCombo("Assign Model Asset", currentLabel.c_str())) {
        for (int i = 0; i < (int)models.size(); ++i) {
            bool sel = (modelIdx == i);
            if (ImGui::Selectable(models[i].c_str(), sel)) {
                modelIdx = i;
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::Button("Assign Selected Model to Entity")) {
        if (modelIdx >= 0 && modelIdx < (int)models.size()) {
            auto* m = AssetManager::getInstance()->getModel(models[modelIdx]);
            if (m) {
                if (e->hasComponent<ModelComponent>()) e->removeComponent<ModelComponent>();
                auto* mc = e->addComponent<ModelComponent>();
                mc->shapes = m->shapes;
            }
        }
    }
}
