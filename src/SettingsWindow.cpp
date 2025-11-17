#include "SettingsWindow.h"

void SettingsWindow::update(EntityManager& em, float deltaTime) {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Settings");
    
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
    ImGui::Text("Window Visibility");
    ImGui::Separator();
    
    ImGui::Checkbox("Performance Monitor", &showPerformanceWindow);
    ImGui::Checkbox("Console", &showConsoleWindow);
    ImGui::Checkbox("Scene Hierarchy", &showSceneHierarchy);
    ImGui::Checkbox("Entity Editor", &showEntityEditor);
    ImGui::Checkbox("Asset Manager", &showAssetManager);
    ImGui::Checkbox("Model Editor", &showModelEditor);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
        "Note: These settings are not yet connected to actual windows.\n"
        "Future update will allow toggling window visibility.");
}

void SettingsWindow::drawRenderingSettings() {
    ImGui::Text("Rendering Options");
    ImGui::Separator();
    
    if (ImGui::Checkbox("VSync", &vsyncEnabled)) {
        // TODO: Implement VSync toggling
        // Would require passing window handle and calling glfwSwapInterval
    }
    
    ImGui::SliderFloat("Target FPS", &targetFPS, 30.0f, 144.0f, "%.0f");
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(Not yet implemented)");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Graphics Quality");
    
    static int qualityPreset = 1; // 0=Low, 1=Medium, 2=High
    ImGui::RadioButton("Low", &qualityPreset, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Medium", &qualityPreset, 1);
    ImGui::SameLine();
    ImGui::RadioButton("High", &qualityPreset, 2);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(Placeholder - not functional)");
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), 
        "Note: Rendering settings require deeper integration\n"
        "with the rendering system and will be implemented\n"
        "in a future update.");
}

void SettingsWindow::drawEditorSettings() {
    ImGui::Text("Editor Options");
    ImGui::Separator();
    
    ImGui::Checkbox("Auto-Save", &autoSaveEnabled);
    
    if (autoSaveEnabled) {
        ImGui::Indent();
        ImGui::SliderInt("Interval (seconds)", &autoSaveInterval, 60, 600);
        ImGui::Text("Next auto-save in: ~%d seconds", autoSaveInterval);
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
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), 
        "These paths are not yet saved between sessions.");
}

void SettingsWindow::drawUISettings() {
    ImGui::Text("UI Appearance");
    ImGui::Separator();
    
    if (ImGui::SliderFloat("UI Scale", &uiScale, 0.5f, 2.0f, "%.1fx")) {
        ImGui::GetIO().FontGlobalScale = uiScale;
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    ImGui::Text("Color Theme:");
    const char* styles[] = { "Dark", "Light", "Classic" };
    if (ImGui::Combo("Style", &styleIndex, styles, IM_ARRAYSIZE(styles))) {
        applyStyle(styleIndex);
    }
    
    ImGui::Spacing();
    
    if (ImGui::Button("Reset to Defaults", ImVec2(150, 0))) {
        uiScale = 1.0f;
        styleIndex = 0;
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
