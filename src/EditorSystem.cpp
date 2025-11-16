#include "EditorSystem.h"
#include "EntitySerializer.h"   // <- neu
#include "AssetManager.h"
#include <cstring> // for strncpy

// temporary path buffers for the UI
static char savePathBuf[256] = "res/entities/entities.json";
static char loadPathBuf[256] = "res/entities/entities.json";

void EditorSystem::update(EntityManager& em, float /*deltaTime*/) {
    ImGui::Begin("Entity Editor");

    // --- Save / Load UI ---
    ImGui::InputText("Save Path", savePathBuf, sizeof(savePathBuf));
    if (ImGui::Button("Save Entities")) {
        bool ok = EntitySerializer::saveEntities(em, std::string(savePathBuf));
        if (ok) ImGui::SameLine(), ImGui::TextColored(ImVec4(0,1,0,1), "Saved");
        else ImGui::SameLine(), ImGui::TextColored(ImVec4(1,0,0,1), "Save failed");
    }
    ImGui::SameLine();
    ImGui::InputText("Load Path", loadPathBuf, sizeof(loadPathBuf));
    if (ImGui::Button("Load Entities")) {
        bool ok = EntitySerializer::loadEntities(em, std::string(loadPathBuf));
        if (ok) ImGui::SameLine(), ImGui::TextColored(ImVec4(0,1,0,1), "Loaded");
        else ImGui::SameLine(), ImGui::TextColored(ImVec4(1,0,0,1), "Load failed");
    }

    ImGui::Separator();

    // --- existing Editor UI follows ---
    // Create entity UI
    ImGui::InputText("Name", newEntityName, sizeof(newEntityName));
    if (ImGui::Button("Create Entity")) {
        Entity* e = em.createEntity();
        e->tag = std::string(newEntityName);
        // Optionally add default components (commented)
        // e->addComponent<TransformComponent>();
        // e->addComponent<RenderComponent>();
        selectedEntityId = e->id;
    }
    ImGui::Separator();

    // Entity list
    ImGui::Text("Entities:");
    ImGui::BeginChild("EntityList", ImVec2(250, 300), true);
    for (auto* e : em.getAllEntities()) {
        char buf[128];
        snprintf(buf, sizeof(buf), "[%u] %s", e->id, e->tag.c_str());
        bool sel = (selectedEntityId == (int)e->id);
        if (ImGui::Selectable(buf, sel)) {
            selectedEntityId = (int)e->id;
        }
    }
    ImGui::EndChild();

    // Destroy selected entity
    if (selectedEntityId >= 0) {
        if (ImGui::Button("Destroy Selected")) {
            em.destroyEntity((unsigned int)selectedEntityId);
            selectedEntityId = -1;
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        // noop - list is built each frame
    }

    ImGui::Separator();

    // Selected entity inspector
    Entity* selected = (selectedEntityId >= 0) ? em.getEntityByID((unsigned int)selectedEntityId) : nullptr;
    if (!selected) {
        ImGui::TextColored(ImVec4(1,0.5f,0,1), "No entity selected");
        ImGui::End();
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

    ImGui::End();
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
