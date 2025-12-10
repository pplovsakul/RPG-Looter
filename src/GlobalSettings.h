#pragma once

// Singleton class to manage global application settings
class GlobalSettings {
public:
    static GlobalSettings& getInstance() {
        static GlobalSettings instance;
        return instance;
    }

    // Window visibility flags
    struct WindowVisibility {
        bool showPerformanceWindow = true;
        bool showConsoleWindow = true;
        bool showSceneHierarchy = true;
        bool showEntityEditor = true;
        bool showAssetManager = true;
        bool showSettingsWindow = true;
        bool showQuickActions = true;
    } windowVisibility;

    // System control flags
    struct SystemControls {
        bool renderingEnabled = true;
        bool audioEnabled = true;
        bool physicsEnabled = true;
    } systemControls;

    // Rendering settings
    struct RenderingSettings {
        bool vsyncEnabled = true;
        float targetFPS = 60.0f;
        int qualityPreset = 1; // 0=Low, 1=Medium, 2=High
    } renderingSettings;

    // Editor settings
    struct EditorSettings {
        bool autoSaveEnabled = false;
        int autoSaveInterval = 300; // seconds
        float autoSaveTimer = 0.0f;
    } editorSettings;

    // UI settings
    struct UISettings {
        float uiScale = 1.0f;
        int styleIndex = 0; // 0=Dark, 1=Light, 2=Classic
    } uiSettings;

    // Delete copy and move constructors
    GlobalSettings(const GlobalSettings&) = delete;
    GlobalSettings& operator=(const GlobalSettings&) = delete;
    GlobalSettings(GlobalSettings&&) = delete;
    GlobalSettings& operator=(GlobalSettings&&) = delete;

private:
    GlobalSettings() = default;
};
