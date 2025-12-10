#pragma once
#include "System.h"
#include "Entitymanager.h"
#include "GlobalSettings.h"
#include "vendor/imgui/imgui.h"
#include <string>

class ModelEditorWindow : public System {
public:
    ModelEditorWindow() = default;
    void update(EntityManager& em, float deltaTime) override;
    
private:
    char modelSearchBuffer[128] = "";
    int selectedModelIndex = -1;
    
    void drawModelList();
    void drawModelDetails();
    void drawModelPreview();
};
