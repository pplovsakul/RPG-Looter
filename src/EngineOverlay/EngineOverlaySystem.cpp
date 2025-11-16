#include "EngineOverlaySystem.h"
#include "Widgets.h"
#include "../Components.h"
#include <iostream>
#include <algorithm>
#include <cstring>

EngineOverlaySystem::EngineOverlaySystem(GLFWwindow* window)
    : m_window(window)
{
    if (!m_window) {
        std::cerr << "EngineOverlaySystem: Invalid window!" << std::endl;
        return;
    }
    
    // Create UI context
    m_uiContext = EngineUI::CreateContext();
    
    // Get window size
    int width, height;
    glfwGetWindowSize(m_window, &width, &height);
    
    // Initialize UI system
    if (!m_uiContext->init(width, height)) {
        std::cerr << "EngineOverlaySystem: Failed to initialize UI context!" << std::endl;
        return;
    }
    
    // Create UI windows
    m_consoleWindow = std::make_unique<EngineUI::ConsoleWindow>();
    m_debugUIWindow = std::make_unique<EngineUI::DebugUIWindow>();
    
    // Initialize frame history for profiler
    m_frameHistory.reserve(120); // 2 seconds at 60fps
    
    m_initialized = true;
    std::cout << "EngineOverlaySystem initialized successfully" << std::endl;
    
    // Add initial console log
    if (m_consoleWindow) {
        m_consoleWindow->addLog("Engine Overlay System initialized", EngineUI::LogEntry::Level::Info);
    }
}

EngineOverlaySystem::~EngineOverlaySystem() {
    if (m_uiContext) {
        EngineUI::DestroyContext(m_uiContext);
        m_uiContext = nullptr;
    }
}

void EngineOverlaySystem::update(EntityManager& em, float deltaTime) {
    if (!m_initialized || !m_visible) {
        return;
    }
    
    // Update profiler stats
    m_frameTimeAccum += deltaTime * 1000.0f; // Convert to ms
    m_frameCount++;
    
    if (m_frameCount >= 10) {
        m_frameTimeMs = m_frameTimeAccum / m_frameCount;
        m_frameTimeAccum = 0.0f;
        m_frameCount = 0;
        
        // Update frame history for graph
        m_frameHistory.push_back(m_frameTimeMs);
        if (m_frameHistory.size() > 120) {
            m_frameHistory.erase(m_frameHistory.begin());
        }
    }
    
    // Handle input
    handleInput();
    
    // Get window size
    int width, height;
    glfwGetWindowSize(m_window, &width, &height);
    
    // Begin UI frame
    m_uiContext->beginFrame(width, height);
    
    // Render UI
    renderUI(em, deltaTime);
    
    // End UI frame
    m_uiContext->endFrame();
    
    // Render to screen
    m_uiContext->render();
}

void EngineOverlaySystem::handleInput() {
    // Get mouse position
    double mouseX, mouseY;
    glfwGetCursorPos(m_window, &mouseX, &mouseY);
    m_uiContext->setMousePos((float)mouseX, (float)mouseY);
    
    // Get mouse buttons
    for (int i = 0; i < 3; ++i) {
        int state = glfwGetMouseButton(m_window, i);
        bool down = (state == GLFW_PRESS);
        m_uiContext->setMouseButton(i, down);
    }
    
    // Check for F1 toggle
    int f1State = glfwGetKey(m_window, GLFW_KEY_F1);
    bool f1Down = (f1State == GLFW_PRESS);
    if (f1Down && !m_prevF1Key) {
        toggleOverlay();
    }
    m_prevF1Key = f1Down;
    
    // Pass keyboard state
    for (int key = 32; key < 348; ++key) {
        int state = glfwGetKey(m_window, key);
        m_uiContext->setKeyState(key, state == GLFW_PRESS);
    }
    
    // TODO: Handle character input via GLFW character callback
}

void EngineOverlaySystem::renderUI(EntityManager& em, float deltaTime) {
    // Render menu bar first
    renderMenuBar();
    
    if (m_showDemo) {
        renderDemoWindow();
    }
    
    if (m_showEntityInspector) {
        renderEntityInspector(em);
    }
    
    if (m_showProfiler) {
        renderProfiler(deltaTime);
    }
    
    if (m_showConsole && m_consoleWindow) {
        m_consoleWindow->render(&m_showConsole);
    }
    
    if (m_showDebugUI && m_debugUIWindow) {
        m_debugUIWindow->render(&m_showDebugUI);
    }
}

void EngineOverlaySystem::renderDemoWindow() {
    using namespace EngineUI;
    
    if (m_uiContext->beginWindow("Engine Overlay Demo")) {
        Text("Welcome to the custom Engine Overlay!");
        Spacing();
        
        Separator();
        Text("Basic Widgets:");
        
        if (Button("Click Me!")) {
            std::cout << "Button clicked!" << std::endl;
        }
        
        static bool checkboxValue = false;
        Checkbox("Checkbox", &checkboxValue);
        
        static bool radioValue = 0;
        if (RadioButton("Option 1", radioValue == 0)) radioValue = 0;
        if (RadioButton("Option 2", radioValue == 1)) radioValue = 1;
        if (RadioButton("Option 3", radioValue == 2)) radioValue = 2;
        
        Separator();
        Text("Sliders:");
        
        static float sliderFloat = 0.5f;
        SliderFloat("Float Slider", &sliderFloat, 0.0f, 1.0f);
        
        static int sliderInt = 50;
        SliderInt("Int Slider", &sliderInt, 0, 100);
        
        Separator();
        Text("Input:");
        
        static char textBuffer[64] = "Edit me!";
        InputText("Text Input", textBuffer, sizeof(textBuffer));
        
        Separator();
        Text("Progress:");
        
        static float progress = 0.7f;
        ProgressBar(progress);
        
        Separator();
        Text("Colors:");
        
        static float color[3] = {1.0f, 0.5f, 0.2f};
        ColorEdit3("Color", color);
        
        Separator();
        Text("Window Controls:");
        Checkbox("Show Entity Inspector", &m_showEntityInspector);
        Checkbox("Show Profiler", &m_showProfiler);
        
        m_uiContext->endWindow();
    }
}

void EngineOverlaySystem::renderEntityInspector(EntityManager& em) {
    using namespace EngineUI;
    
    EngineUI::Rect initialRect(500, 100, 350, 400);
    if (m_uiContext->beginWindow("Entity Inspector", nullptr, &initialRect)) {
        Text("Entity Management");
        Separator();
        
        auto entities = em.getAllEntities();
        
        char countText[64];
        snprintf(countText, sizeof(countText), "Total Entities: %zu", entities.size());
        Text(countText);
        
        Spacing();
        
        if (Button("Create Entity")) {
            Entity* e = em.createEntity();
            e->tag = "New Entity";
            std::cout << "Created entity #" << e->id << std::endl;
        }
        
        Separator();
        Text("Entity List:");
        
        for (Entity* e : entities) {
            if (e) {
                char labelBuf[128];
                snprintf(labelBuf, sizeof(labelBuf), "[%u] %s", e->id, e->tag.c_str());
                
                if (CollapsingHeader(labelBuf)) {
                    Indent();
                    
                    char idText[64];
                    snprintf(idText, sizeof(idText), "ID: %u", e->id);
                    Text(idText);
                    
                    // Show components
                    if (e->hasComponent<TransformComponent>()) {
                        Text("Has Transform");
                    }
                    if (e->hasComponent<RenderComponent>()) {
                        Text("Has Render");
                    }
                    if (e->hasComponent<AudioComponent>()) {
                        Text("Has Audio");
                    }
                    
                    if (Button("Destroy")) {
                        em.destroyEntity(e->id);
                    }
                    
                    Unindent();
                }
            }
        }
        
        m_uiContext->endWindow();
    }
}

void EngineOverlaySystem::renderProfiler(float deltaTime) {
    using namespace EngineUI;
    
    EngineUI::Rect initialRect(900, 100, 300, 250);
    if (m_uiContext->beginWindow("Profiler", nullptr, &initialRect)) {
        Text("Performance Monitor");
        Separator();
        
        char fpsText[64];
        float fps = (m_frameTimeMs > 0.0f) ? (1000.0f / m_frameTimeMs) : 0.0f;
        snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps);
        Text(fpsText);
        
        char frameTimeText[64];
        snprintf(frameTimeText, sizeof(frameTimeText), "Frame Time: %.2f ms", m_frameTimeMs);
        Text(frameTimeText);
        
        char deltaText[64];
        snprintf(deltaText, sizeof(deltaText), "Delta Time: %.4f s", deltaTime);
        Text(deltaText);
        
        Separator();
        
        // Show frame time as progress bar (normalized to 16.67ms for 60fps)
        float targetFrameTime = 16.67f;
        float fraction = m_frameTimeMs / targetFrameTime;
        ProgressBar(fraction);
        
        if (fraction < 0.8f) {
            TextColored(Color::Green(), "Performance: Excellent");
        } else if (fraction < 1.2f) {
            TextColored(Color::Yellow(), "Performance: Good");
        } else {
            TextColored(Color::Red(), "Performance: Poor");
        }
        
        Separator();
        
        // Frame time graph
        Text("Frame Time History:");
        if (!m_frameHistory.empty()) {
            // Find min/max for scaling
            float minTime = m_frameHistory[0];
            float maxTime = m_frameHistory[0];
            for (float t : m_frameHistory) {
                minTime = std::min(minTime, t);
                maxTime = std::max(maxTime, t);
            }
            
            char graphText[128];
            snprintf(graphText, sizeof(graphText), "Range: %.2f - %.2f ms (%zu samples)", 
                    minTime, maxTime, m_frameHistory.size());
            Text(graphText);
            
            // Simple text-based visualization
            Text("Graph: (120 samples, newest on right)");
            char graphLine[128] = "";
            for (size_t i = 0; i < std::min(m_frameHistory.size(), (size_t)60); ++i) {
                size_t idx = m_frameHistory.size() > 60 ? m_frameHistory.size() - 60 + i : i;
                float t = m_frameHistory[idx];
                
                // Scale to 0-9 range
                int barHeight = (int)((t / targetFrameTime) * 5.0f);
                barHeight = std::max(0, std::min(9, barHeight));
                
                char c = '0' + barHeight;
                strncat(graphLine, &c, 1);
            }
            Text(graphLine);
        }
        
        m_uiContext->endWindow();
    }
}

void EngineOverlaySystem::renderMenuBar() {
    using namespace EngineUI;
    
    // Simple menu bar at top of screen
    EngineUI::Rect menuRect(0, 0, 1920, 24); // Fixed width, will clip
    if (m_uiContext->beginWindow("##MenuBar", nullptr, &menuRect)) {
        Text("View:");
        SameLine();
        
        if (Button("Demo")) {
            m_showDemo = !m_showDemo;
        }
        SameLine();
        
        if (Button("Inspector")) {
            m_showEntityInspector = !m_showEntityInspector;
        }
        SameLine();
        
        if (Button("Profiler")) {
            m_showProfiler = !m_showProfiler;
        }
        SameLine();
        
        if (Button("Console")) {
            m_showConsole = !m_showConsole;
        }
        SameLine();
        
        if (Button("Debug UI")) {
            m_showDebugUI = !m_showDebugUI;
        }
        SameLine();
        
        Text("|");
        SameLine();
        
        if (Button("Hide All (F1)")) {
            toggleOverlay();
        }
        
        m_uiContext->endWindow();
    }
}

void EngineOverlaySystem::toggleOverlay() {
    m_visible = !m_visible;
    std::cout << "Overlay " << (m_visible ? "shown" : "hidden") << std::endl;
}

bool EngineOverlaySystem::wantsCaptureMouse() const {
    // TODO: Query UI context for mouse capture
    return m_visible && m_initialized;
}

bool EngineOverlaySystem::wantsCaptureKeyboard() const {
    // TODO: Query UI context for keyboard capture
    return m_visible && m_initialized;
}

void EngineOverlaySystem::onWindowResize(int width, int height) {
    if (m_uiContext) {
        // The context will handle this in the next frame
    }
}
