#pragma once

#include "../System.h"
#include "../Entitymanager.h"
#include "UIContext.h"
#include "ConsoleWindow.h"
#include "DebugUIWindow.h"
#include <GLFW/glfw3.h>
#include <memory>

/**
 * EngineOverlaySystem - Integrates custom UI system into the game
 */
class EngineOverlaySystem : public System {
public:
    EngineOverlaySystem(GLFWwindow* window);
    ~EngineOverlaySystem();
    
    void update(EntityManager& em, float deltaTime) override;
    
    // Visibility
    void toggleOverlay();
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }
    
    // Input capture
    bool wantsCaptureMouse() const;
    bool wantsCaptureKeyboard() const;
    
    void onWindowResize(int width, int height);
    
private:
    void handleInput();
    void renderUI(EntityManager& em, float deltaTime);
    void renderMenuBar();
    void renderDemoWindow();
    void renderEntityInspector(EntityManager& em);
    void renderProfiler(float deltaTime);
    
    GLFWwindow* m_window = nullptr;
    EngineUI::UIContext* m_uiContext = nullptr;
    
    bool m_visible = true;
    bool m_initialized = false;
    
    // Window visibility
    bool m_showDemo = true;
    bool m_showEntityInspector = true;
    bool m_showProfiler = true;
    bool m_showConsole = true;
    bool m_showDebugUI = true;
    
    // Profiler data
    float m_frameTimeMs = 0.0f;
    float m_frameTimeAccum = 0.0f;
    int m_frameCount = 0;
    std::vector<float> m_frameHistory;
    
    // Input state (previous frame)
    bool m_prevF1Key = false;
    bool m_prevMouseButtons[5] = {false};
    double m_prevMouseX = 0.0;
    double m_prevMouseY = 0.0;
    
    // UI Windows
    std::unique_ptr<EngineUI::ConsoleWindow> m_consoleWindow;
    std::unique_ptr<EngineUI::DebugUIWindow> m_debugUIWindow;
};
