#include "SceneHierarchyWindow.h"
#include "Components.h"
#include <algorithm>
#include <map>

void SceneHierarchyWindow::update(EntityManager& em, float deltaTime) {
    ImGui::SetNextWindowPos(ImVec2(10, 320), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("Scene Hierarchy");
    
    // Search bar
    ImGui::InputTextWithHint("##search", "Search entities...", searchBuffer, sizeof(searchBuffer));
    
    // Options
    ImGui::Checkbox("Group by Tag", &groupByTag);
    ImGui::SameLine();
    ImGui::Checkbox("Only with Components", &showOnlyWithComponents);
    
    ImGui::Separator();
    
    // Entity count
    auto entities = em.getAllEntities();
    ImGui::Text("Total Entities: %zu", entities.size());
    
    ImGui::Spacing();
    
    // Entity tree
    ImGui::BeginChild("EntityTree", ImVec2(0, -30), true);
    drawEntityTree(em);
    ImGui::EndChild();
    
    // Bottom controls
    ImGui::Separator();
    if (ImGui::Button("Create Entity")) {
        Entity* e = em.createEntity();
        e->tag = "NewEntity";
        selectedEntityId = e->id;
    }
    
    ImGui::SameLine();
    if (selectedEntityId >= 0) {
        if (ImGui::Button("Delete Selected")) {
            em.destroyEntity((unsigned int)selectedEntityId);
            selectedEntityId = -1;
        }
    }
    
    ImGui::End();
}

void SceneHierarchyWindow::drawEntityTree(EntityManager& em) {
    auto entities = em.getAllEntities();
    
    if (groupByTag) {
        // Group entities by tag
        std::map<std::string, std::vector<Entity*>> groupedEntities;
        for (auto* entity : entities) {
            // Apply filters
            if (searchBuffer[0] != '\0' && !matchesSearch(entity->tag)) continue;
            if (showOnlyWithComponents && !entity->hasComponent<TransformComponent>()) continue;
            
            std::string tag = entity->tag.empty() ? "(No Tag)" : entity->tag;
            groupedEntities[tag].push_back(entity);
        }
        
        // Display grouped entities
        for (const auto& [tag, group] : groupedEntities) {
            if (ImGui::TreeNode(tag.c_str())) {
                for (auto* entity : group) {
                    char label[128];
                    snprintf(label, sizeof(label), "[%u] %s", entity->id, entity->tag.c_str());
                    
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                    if (selectedEntityId == (int)entity->id) {
                        flags |= ImGuiTreeNodeFlags_Selected;
                    }
                    
                    ImGui::TreeNodeEx((void*)(intptr_t)entity->id, flags, "%s", label);
                    
                    if (ImGui::IsItemClicked()) {
                        selectedEntityId = entity->id;
                    }
                    
                    // Right-click context menu
                    if (ImGui::BeginPopupContextItem()) {
                        drawEntityContextMenu(entity, em);
                        ImGui::EndPopup();
                    }
                }
                ImGui::TreePop();
            }
        }
    } else {
        // Flat list
        for (auto* entity : entities) {
            // Apply filters
            if (searchBuffer[0] != '\0' && !matchesSearch(entity->tag)) continue;
            if (showOnlyWithComponents && !entity->hasComponent<TransformComponent>()) continue;
            
            char label[128];
            snprintf(label, sizeof(label), "[%u] %s", entity->id, entity->tag.c_str());
            
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            if (selectedEntityId == (int)entity->id) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            
            // Add icon based on components
            std::string icon = "";
            if (entity->hasComponent<RenderComponent>()) icon += "[R]";
            if (entity->hasComponent<TransformComponent>()) icon += "[T]";
            if (entity->hasComponent<AudioComponent>()) icon += "[A]";
            if (entity->hasComponent<ModelComponent>()) icon += "[M]";
            
            std::string fullLabel = icon + " " + label;
            
            ImGui::TreeNodeEx((void*)(intptr_t)entity->id, flags, "%s", fullLabel.c_str());
            
            if (ImGui::IsItemClicked()) {
                selectedEntityId = entity->id;
            }
            
            // Right-click context menu
            if (ImGui::BeginPopupContextItem()) {
                drawEntityContextMenu(entity, em);
                ImGui::EndPopup();
            }
        }
    }
}

void SceneHierarchyWindow::drawEntityContextMenu(Entity* entity, EntityManager& em) {
    ImGui::Text("Entity: %u", entity->id);
    ImGui::Separator();
    
    if (ImGui::MenuItem("Select")) {
        selectedEntityId = entity->id;
    }
    
    if (ImGui::MenuItem("Duplicate")) {
        Entity* newEntity = em.createEntity();
        newEntity->tag = entity->tag + "_copy";
        
        // Copy components (basic copy)
        if (entity->hasComponent<TransformComponent>()) {
            auto* src = entity->getComponent<TransformComponent>();
            auto* dst = newEntity->addComponent<TransformComponent>();
            *dst = *src;
        }
        if (entity->hasComponent<RenderComponent>()) {
            auto* src = entity->getComponent<RenderComponent>();
            auto* dst = newEntity->addComponent<RenderComponent>();
            *dst = *src;
        }
    }
    
    if (ImGui::MenuItem("Delete", "Del")) {
        em.destroyEntity(entity->id);
        if (selectedEntityId == (int)entity->id) {
            selectedEntityId = -1;
        }
    }
    
    ImGui::Separator();
    
    // Quick component toggles
    if (entity->hasComponent<TransformComponent>()) {
        if (ImGui::MenuItem("Remove Transform")) {
            entity->removeComponent<TransformComponent>();
        }
    } else {
        if (ImGui::MenuItem("Add Transform")) {
            entity->addComponent<TransformComponent>();
        }
    }
    
    if (entity->hasComponent<RenderComponent>()) {
        if (ImGui::MenuItem("Remove Render")) {
            entity->removeComponent<RenderComponent>();
        }
    } else {
        if (ImGui::MenuItem("Add Render")) {
            entity->addComponent<RenderComponent>();
        }
    }
}

bool SceneHierarchyWindow::matchesSearch(const std::string& text) {
    if (searchBuffer[0] == '\0') return true;
    
    std::string search = searchBuffer;
    std::string textLower = text;
    
    // Simple case-insensitive search
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
    std::transform(textLower.begin(), textLower.end(), textLower.begin(), ::tolower);
    
    return textLower.find(search) != std::string::npos;
}
