#pragma once
#include "System.h"
#include "Entitymanager.h"
#include "vendor/imgui/imgui.h"
#include <string>

class SceneHierarchyWindow : public System {
public:
    SceneHierarchyWindow() = default;
    void update(EntityManager& em, float deltaTime) override;
    
    // Get selected entity ID (for integration with other windows)
    int getSelectedEntityId() const { return selectedEntityId; }
    void setSelectedEntityId(int id) { selectedEntityId = id; }
    
private:
    int selectedEntityId = -1;
    char searchBuffer[128] = "";
    bool groupByTag = false;
    bool showOnlyWithComponents = false;
    
    void drawEntityTree(EntityManager& em);
    void drawEntityContextMenu(Entity* entity, EntityManager& em);
    bool matchesSearch(const std::string& text);
};
