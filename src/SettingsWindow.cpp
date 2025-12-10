#include "SettingsWindow.h"

void SettingsWindow::update(EntityManager& em, float deltaTime) {
    auto& settings = GlobalSettings::getInstance();
    
    // Only show if the window is visible
    if (!settings.windowVisibility.showSettingsWindow) {
        return;
    }
    
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Settings", &settings.windowVisibility.showSettingsWindow)) {
        ImGui::End();
        return;
    }
    
    if (ImGui::BeginTabBar("SettingsTabs")) {
        if (ImGui::BeginTabItem("Windows")) {
            drawWindowSettings();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Rendering")) {
            drawRenderingSettings();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Editor")) {
            drawEditorSettings();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("UI")) {
            drawUISettings();
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
    
    ImGui::End();
}

void SettingsWindow::drawWindowSettings() {
    auto& settings = GlobalSettings::getInstance();
    
    ImGui::Text("Window Visibility");
    ImGui::Separator();
    
    ImGui::Checkbox("Performance Monitor", &settings.windowVisibility.showPerformanceWindow);
    ImGui::Checkbox("Console", &settings.windowVisibility.showConsoleWindow);
    ImGui::Checkbox("Scene Hierarchy", &settings.windowVisibility.showSceneHierarchy);
    ImGui::Checkbox("Entity Editor", &settings.windowVisibility.showEntityEditor);
    ImGui::Checkbox("Asset Manager", &settings.windowVisibility.showAssetManager);
    ImGui::Checkbox("Quick Actions", &settings.windowVisibility.showQuickActions);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), 
        "Window visibility settings are now functional!");
}

void SettingsWindow::drawRenderingSettings() {
    auto& settings = GlobalSettings::getInstance();
    
    ImGui::Text("Rendering Options");
    ImGui::Separator();
    
    if (ImGui::Checkbox("VSync", &settings.renderingSettings.vsyncEnabled)) {
        // Note: VSync will be applied in the main loop via Game::getVSyncEnabled()
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), 
            "VSync change will take effect on next frame");
    }
    
    ImGui::SliderFloat("Target FPS", &settings.renderingSettings.targetFPS, 30.0f, 144.0f, "%.0f");
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "FPS limiting is active");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Graphics Quality");
    
    ImGui::RadioButton("Low", &settings.renderingSettings.qualityPreset, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Medium", &settings.renderingSettings.qualityPreset, 1);
    ImGui::SameLine();
    ImGui::RadioButton("High", &settings.renderingSettings.qualityPreset, 2);
    
    const char* qualityDesc[] = {
        "Low: Optimized for performance",
        "Medium: Balanced quality and performance",
        "High: Best visual quality"
    };
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", 
        qualityDesc[settings.renderingSettings.qualityPreset]);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), 
        "VSync and FPS limiting are now functional!\n"
        "Quality presets are available for future use.");
}

void SettingsWindow::drawEditorSettings() {
    auto& settings = GlobalSettings::getInstance();
    
    ImGui::Text("Editor Options");
    ImGui::Separator();
    
    ImGui::Checkbox("Auto-Save", &settings.editorSettings.autoSaveEnabled);
    
    if (settings.editorSettings.autoSaveEnabled) {
        ImGui::Indent();
        ImGui::SliderInt("Interval (seconds)", &settings.editorSettings.autoSaveInterval, 60, 600);
        int remainingTime = settings.editorSettings.autoSaveInterval - (int)settings.editorSettings.autoSaveTimer;
        if (remainingTime < 0) remainingTime = 0;
        ImGui::Text("Next auto-save in: %d seconds", remainingTime);
        ImGui::Unindent();
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    ImGui::Text("Default Paths:");
    static char entityPath[256] = "res/entities/";
    static char modelPath[256] = "res/models/";
    static char texturePath[256] = "res/textures/";
    
    ImGui::InputText("Entities", entityPath, sizeof(entityPath));
    ImGui::InputText("Models", modelPath, sizeof(modelPath));
    ImGui::InputText("Textures", texturePath, sizeof(texturePath));
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), 
        "Auto-save timer is now functional!\n"
        "Path settings can be saved in future updates.");
}

void SettingsWindow::drawUISettings() {
    auto& settings = GlobalSettings::getInstance();
    
    ImGui::Text("UI Appearance");
    ImGui::Separator();
    
    if (ImGui::SliderFloat("UI Scale", &settings.uiSettings.uiScale, 0.5f, 2.0f, "%.1fx")) {
        ImGui::GetIO().FontGlobalScale = settings.uiSettings.uiScale;
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    ImGui::Text("Color Theme:");
    const char* styles[] = { "Dark", "Light", "Classic" };
    if (ImGui::Combo("Style", &settings.uiSettings.styleIndex, styles, IM_ARRAYSIZE(styles))) {
        applyStyle(settings.uiSettings.styleIndex);
    }
    
    ImGui::Spacing();
    
    if (ImGui::Button("Reset to Defaults", ImVec2(150, 0))) {
        settings.uiSettings.uiScale = 1.0f;
        settings.uiSettings.styleIndex = 0;
        ImGui::GetIO().FontGlobalScale = 1.0f;
        applyStyle(0);
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    ImGui::Text("ImGui Demo Window:");
    static bool showDemoWindow = false;
    ImGui::Checkbox("Show ImGui Demo", &showDemoWindow);
    
    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
}

void SettingsWindow::applyStyle(int index) {
    switch (index) {
        case 0: // Dark
            ImGui::StyleColorsDark();
            break;
        case 1: // Light
            ImGui::StyleColorsLight();
            break;
        case 2: // Classic
            ImGui::StyleColorsClassic();
            break;
    }
}
