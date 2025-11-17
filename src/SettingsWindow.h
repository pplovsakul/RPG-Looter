#pragma once
#include "System.h"
#include "Entitymanager.h"
#include "vendor/imgui/imgui.h"

class SettingsWindow : public System {
public:
    SettingsWindow() = default;
    void update(EntityManager& em, float deltaTime) override;
    
private:
    // Window visibility settings
    bool showPerformanceWindow = true;
    bool showConsoleWindow = true;
    bool showSceneHierarchy = true;
    bool showEntityEditor = true;
    bool showAssetManager = true;
    bool showModelEditor = true;
    
    // Rendering settings
    bool vsyncEnabled = true;
    float targetFPS = 60.0f;
    
    // Editor settings
    bool autoSaveEnabled = false;
    int autoSaveInterval = 300; // seconds
    
    // UI settings
    float uiScale = 1.0f;
    int styleIndex = 0; // 0=Dark, 1=Light, 2=Classic
    
    void drawWindowSettings();
    void drawRenderingSettings();
    void drawEditorSettings();
    void drawUISettings();
    void applyStyle(int index);
};
