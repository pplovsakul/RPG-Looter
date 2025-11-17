#pragma once
#include "System.h"
#include "Entitymanager.h"
#include "GlobalSettings.h"
#include "vendor/imgui/imgui.h"

class SettingsWindow : public System {
public:
    SettingsWindow() = default;
    void update(EntityManager& em, float deltaTime) override;
    
private:
    void drawWindowSettings();
    void drawRenderingSettings();
    void drawEditorSettings();
    void drawUISettings();
    void applyStyle(int index);
};
