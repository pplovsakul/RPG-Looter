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
    std::cout << "Professional Engine Overlay initialized (Unity/Unreal style)" << std::endl;
    
    // Add initial console log
    if (m_consoleWindow) {
        m_consoleWindow->addLog("=== RPG-Looter Engine Started ===", EngineUI::LogEntry::Level::Info);
        m_consoleWindow->addLog("Custom Engine Overlay Ready - Professional Mode", EngineUI::LogEntry::Level::Info);
        m_consoleWindow->addLog("Press F1 to toggle overlay visibility", EngineUI::LogEntry::Level::Info);
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
    // Get window size
    int width, height;
    glfwGetWindowSize(m_window, &width, &height);
    
    // Render main menu bar (Unity/Unreal style)
    renderMenuBar(width);
    
    // Render toolbar below menu bar
    renderToolbar(width);
    
    // Render Hierarchy window (left side - like Unity)
    if (m_showEntityInspector) {
        renderHierarchyWindow(em);
    }
    
    // Render Inspector window (right side - like Unity)
    if (m_showEntityInspector) {
        renderInspectorWindow(em);
    }
    
    // Render Scene Stats window (top right)
    if (m_showProfiler) {
        renderSceneStatsWindow(deltaTime);
    }
    
    // Render Console window (bottom - like Unity/Unreal)
    if (m_showConsole && m_consoleWindow) {
        renderConsoleWindowProfessional(&m_showConsole);
    }
    
    // Optional: Debug UI for diagnostics
    if (m_showDebugUI && m_debugUIWindow) {
        m_debugUIWindow->render(&m_showDebugUI);
    }
}

void EngineOverlaySystem::renderToolbar(int screenWidth) {
    using namespace EngineUI;
    
    // Toolbar below menu bar (Unity/Unreal style)
    EngineUI::Rect toolbarRect(0, 26, (float)screenWidth, 40);
    if (m_uiContext->beginWindow("##Toolbar", nullptr, &toolbarRect)) {
        // Play/Pause/Step controls (centered)
        SameLine(screenWidth / 2.0f - 150.0f);
        
        static bool playMode = false;
        if (Button(playMode ? "||" : ">", 50, 30)) {
            playMode = !playMode;
            if (m_consoleWindow) {
                m_consoleWindow->addLog(playMode ? "Entering Play Mode" : "Exiting Play Mode", 
                                       EngineUI::LogEntry::Level::Info);
            }
        }
        
        SameLine();
        if (Button("||>", 50, 30)) {
            if (m_consoleWindow) {
                m_consoleWindow->addLog("Step Frame (not implemented)", EngineUI::LogEntry::Level::Warning);
            }
        }
        
        SameLine();
        if (Button("[]", 50, 30)) {
            playMode = false;
            if (m_consoleWindow) {
                m_consoleWindow->addLog("Stop", EngineUI::LogEntry::Level::Info);
            }
        }
        
        m_uiContext->endWindow();
    }
}

void EngineOverlaySystem::renderHierarchyWindow(EntityManager& em) {
    using namespace EngineUI;
    
    // Hierarchy window on the left (Unity style)
    EngineUI::Rect hierarchyRect(10, 75, 320, 450);
    if (m_uiContext->beginWindow("Hierarchy", nullptr, &hierarchyRect)) {
        Text("Scene Objects");
        Separator();
        
        auto entities = em.getAllEntities();
        
        // Entity count header
        char countText[64];
        snprintf(countText, sizeof(countText), "Total Entities: %zu", entities.size());
        TextColored(Color(0.4f, 0.4f, 0.4f, 1.0f), countText);
        
        Spacing();
        
        // Add new entity button
        if (Button("+ Create Entity", 150, 25)) {
            Entity* e = em.createEntity();
            e->tag = "New Entity";
            e->addComponent<TransformComponent>();
            m_selectedEntity = e;
            if (m_consoleWindow) {
                m_consoleWindow->addLog("Created new entity", EngineUI::LogEntry::Level::Info);
            }
        }
        
        Separator();
        
        // Entity list
        for (Entity* entity : entities) {
            if (!entity) continue;
            
            char labelBuf[256];
            snprintf(labelBuf, sizeof(labelBuf), "%s (ID:%u)", entity->tag.c_str(), entity->id);
            
            // Highlight selected entity
            bool isSelected = (m_selectedEntity == entity);
            
            if (isSelected) {
                // Draw selection background
                auto rect = m_uiContext->allocRect(300, 20);
                m_uiContext->getDrawList().addRectFilled(rect, Color(1.0f, 0.85f, 0.3f, 0.4f));
            }
            
            // Entity name (clickable)
            if (Button(labelBuf, 300, 20)) {
                m_selectedEntity = entity;
            }
            
            // Component icons
            SameLine();
            if (entity->hasComponent<TransformComponent>()) {
                TextColored(Color::Green(), "[T]");
                SameLine();
            }
            if (entity->hasComponent<RenderComponent>()) {
                TextColored(Color::Blue(), "[R]");
                SameLine();
            }
            if (entity->hasComponent<AudioComponent>()) {
                TextColored(Color::Yellow(), "[A]");
            }
        }
        
        m_uiContext->endWindow();
    }
}

void EngineOverlaySystem::renderInspectorWindow(EntityManager& em) {
    using namespace EngineUI;
    
    // Inspector window on the right (Unity style)
    int screenWidth, screenHeight;
    glfwGetWindowSize(m_window, &screenWidth, &screenHeight);
    
    EngineUI::Rect inspectorRect(screenWidth - 330, 75, 320, 450);
    if (m_uiContext->beginWindow("Inspector", nullptr, &inspectorRect)) {
        if (m_selectedEntity) {
            // Entity header
            TextColored(Color(0.8f, 0.7f, 0.2f, 1.0f), m_selectedEntity->tag.c_str());
            
            char idText[32];
            snprintf(idText, sizeof(idText), "ID: %u", m_selectedEntity->id);
            TextColored(Color(0.5f, 0.5f, 0.5f, 1.0f), idText);
            
            Separator();
            
            // Tag editor
            static char tagBuf[128];
            strncpy(tagBuf, m_selectedEntity->tag.c_str(), sizeof(tagBuf) - 1);
            tagBuf[sizeof(tagBuf) - 1] = '\0';
            
            Text("Tag:");
            if (InputText("##tag", tagBuf, sizeof(tagBuf))) {
                m_selectedEntity->tag = tagBuf;
            }
            
            Separator();
            
            // Transform Component
            if (m_selectedEntity->hasComponent<TransformComponent>()) {
                if (CollapsingHeader("Transform Component")) {
                    auto* transform = m_selectedEntity->getComponent<TransformComponent>();
                    
                    Text("Position:");
                    DragFloat("X##pos", &transform->position.x, 1.0f);
                    DragFloat("Y##pos", &transform->position.y, 1.0f);
                    
                    Spacing();
                    Text("Scale:");
                    DragFloat("X##scale", &transform->scale.x, 0.1f);
                    DragFloat("Y##scale", &transform->scale.y, 0.1f);
                    
                    Spacing();
                    Text("Rotation:");
                    DragFloat("##rotation", &transform->rotation, 0.5f);
                }
            } else {
                if (Button("+ Add Transform Component", 280, 25)) {
                    m_selectedEntity->addComponent<TransformComponent>();
                    if (m_consoleWindow) {
                        m_consoleWindow->addLog("Added Transform component", EngineUI::LogEntry::Level::Info);
                    }
                }
            }
            
            Separator();
            
            // Render Component
            if (m_selectedEntity->hasComponent<RenderComponent>()) {
                if (CollapsingHeader("Render Component")) {
                    auto* render = m_selectedEntity->getComponent<RenderComponent>();
                    
                    Text("Color:");
                    ColorEdit3("##color", &render->color.r);
                    
                    Text("Alpha:");
                    SliderFloat("##alpha", &render->alpha, 0.0f, 1.0f);
                    
                    Text("Render Layer:");
                    DragInt("##layer", &render->renderLayer, 1.0f);
                    
                    Text("Shader:");
                    static char shaderBuf[64];
                    strncpy(shaderBuf, render->shaderName.c_str(), sizeof(shaderBuf) - 1);
                    shaderBuf[sizeof(shaderBuf) - 1] = '\0';
                    if (InputText("##shader", shaderBuf, sizeof(shaderBuf))) {
                        render->shaderName = shaderBuf;
                    }
                }
            } else {
                if (Button("+ Add Render Component", 280, 25)) {
                    m_selectedEntity->addComponent<RenderComponent>();
                    auto* render = m_selectedEntity->getComponent<RenderComponent>();
                    render->setMesh("quad");
                    render->shaderName = "default";
                    if (m_consoleWindow) {
                        m_consoleWindow->addLog("Added Render component", EngineUI::LogEntry::Level::Info);
                    }
                }
            }
            
            Separator();
            
            // Audio Component
            if (m_selectedEntity->hasComponent<AudioComponent>()) {
                if (CollapsingHeader("Audio Component")) {
                    TextColored(Color(0.6f, 0.6f, 0.6f, 1.0f), "Audio component present");
                    Text("(Audio controls not implemented)");
                }
            } else {
                if (Button("+ Add Audio Component", 280, 25)) {
                    m_selectedEntity->addComponent<AudioComponent>();
                    if (m_consoleWindow) {
                        m_consoleWindow->addLog("Added Audio component", EngineUI::LogEntry::Level::Info);
                    }
                }
            }
            
            Separator();
            
            // Delete entity button (red colored)
            if (Button("Delete Entity", 280, 30)) {
                em.destroyEntity(m_selectedEntity->id);
                m_selectedEntity = nullptr;
                if (m_consoleWindow) {
                    m_consoleWindow->addLog("Entity deleted", EngineUI::LogEntry::Level::Info);
                }
            }
            
        } else {
            // No entity selected
            Spacing();
            Spacing();
            TextColored(Color(0.5f, 0.5f, 0.5f, 1.0f), "No entity selected");
            Spacing();
            Text("Select an entity from the");
            Text("Hierarchy to inspect it.");
        }
        
        m_uiContext->endWindow();
    }
}

void EngineOverlaySystem::renderSceneStatsWindow(float deltaTime) {
    using namespace EngineUI;
    
    // Stats window (top right, below toolbar)
    int screenWidth, screenHeight;
    glfwGetWindowSize(m_window, &screenWidth, &screenHeight);
    
    EngineUI::Rect statsRect(screenWidth - 330, 535, 320, 250);
    if (m_uiContext->beginWindow("Scene Stats", nullptr, &statsRect)) {
        TextColored(Color(0.8f, 0.7f, 0.2f, 1.0f), "Performance Monitor");
        Separator();
        
        // FPS
        char fpsText[64];
        float fps = (m_frameTimeMs > 0.0f) ? (1000.0f / m_frameTimeMs) : 0.0f;
        snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps);
        Text(fpsText);
        
        // Frame time
        char frameTimeText[64];
        snprintf(frameTimeText, sizeof(frameTimeText), "Frame Time: %.2f ms", m_frameTimeMs);
        Text(frameTimeText);
        
        // Delta time
        char deltaText[64];
        snprintf(deltaText, sizeof(deltaText), "Delta Time: %.4f s", deltaTime);
        Text(deltaText);
        
        Separator();
        
        // Performance indicator
        float targetFrameTime = 16.67f; // 60 FPS target
        float fraction = m_frameTimeMs / targetFrameTime;
        ProgressBar(std::min(fraction, 1.0f));
        
        if (fraction < 0.8f) {
            TextColored(Color::Green(), "Status: Excellent");
        } else if (fraction < 1.2f) {
            TextColored(Color::Yellow(), "Status: Good");
        } else {
            TextColored(Color::Red(), "Status: Poor");
        }
        
        Separator();
        
        // Frame time graph
        if (!m_frameHistory.empty()) {
            Text("Frame Time History:");
            
            // Find min/max for display
            float minTime = m_frameHistory[0];
            float maxTime = m_frameHistory[0];
            for (float t : m_frameHistory) {
                minTime = std::min(minTime, t);
                maxTime = std::max(maxTime, t);
            }
            
            char rangeText[128];
            snprintf(rangeText, sizeof(rangeText), "Range: %.1f - %.1f ms", minTime, maxTime);
            TextColored(Color(0.5f, 0.5f, 0.5f, 1.0f), rangeText);
            
            // Simple text-based graph visualization
            char graphLine[64] = "";
            for (size_t i = 0; i < std::min(m_frameHistory.size(), (size_t)50); ++i) {
                size_t idx = m_frameHistory.size() > 50 ? m_frameHistory.size() - 50 + i : i;
                float t = m_frameHistory[idx];
                
                // Scale to 0-9 range
                int barHeight = (int)((t / targetFrameTime) * 5.0f);
                barHeight = std::max(0, std::min(9, barHeight));
                
                char c = '0' + barHeight;
                strncat(graphLine, &c, 1);
            }
            TextColored(Color::Green(), graphLine);
        }
        
        m_uiContext->endWindow();
    }
}

void EngineOverlaySystem::renderConsoleWindowProfessional(bool* p_open) {
    using namespace EngineUI;
    
    // Console window at bottom (Unity/Unreal style)
    int screenWidth, screenHeight;
    glfwGetWindowSize(m_window, &screenWidth, &screenHeight);
    
    EngineUI::Rect consoleRect(10, screenHeight - 260, screenWidth - 20, 250);
    if (m_consoleWindow) {
        m_consoleWindow->render(p_open, &consoleRect);
    }
}

void EngineOverlaySystem::renderMenuBar(int screenWidth) {
    using namespace EngineUI;
    
    // Professional menu bar at top of screen (Unity/Unreal style)
    EngineUI::Rect menuRect(0, 0, (float)screenWidth, 26);
    if (m_uiContext->beginWindow("##MainMenuBar", nullptr, &menuRect)) {
        // File Menu
        Text("File");
        SameLine();
        if (Button("New")) {
            if (m_consoleWindow) {
                m_consoleWindow->addLog("File > New Scene (not implemented)", EngineUI::LogEntry::Level::Warning);
            }
        }
        SameLine();
        if (Button("Open")) {
            if (m_consoleWindow) {
                m_consoleWindow->addLog("File > Open Scene (not implemented)", EngineUI::LogEntry::Level::Warning);
            }
        }
        SameLine();
        if (Button("Save")) {
            if (m_consoleWindow) {
                m_consoleWindow->addLog("File > Save Scene (not implemented)", EngineUI::LogEntry::Level::Warning);
            }
        }
        
        SameLine();
        Text("|");
        
        // Edit Menu
        SameLine();
        Text("Edit");
        SameLine();
        if (Button("Undo")) {
            if (m_consoleWindow) {
                m_consoleWindow->addLog("Edit > Undo (not implemented)", EngineUI::LogEntry::Level::Warning);
            }
        }
        SameLine();
        if (Button("Redo")) {
            if (m_consoleWindow) {
                m_consoleWindow->addLog("Edit > Redo (not implemented)", EngineUI::LogEntry::Level::Warning);
            }
        }
        
        SameLine();
        Text("|");
        
        // GameObject Menu
        SameLine();
        Text("GameObject");
        SameLine();
        if (Button("Create Empty")) {
            // Will be handled in a submenu
        }
        
        SameLine();
        Text("|");
        
        // Window Menu
        SameLine();
        Text("Window");
        SameLine();
        if (Button("Hierarchy")) {
            m_showEntityInspector = !m_showEntityInspector;
        }
        SameLine();
        if (Button("Console")) {
            m_showConsole = !m_showConsole;
        }
        SameLine();
        if (Button("Stats")) {
            m_showProfiler = !m_showProfiler;
        }
        
        SameLine();
        Text("|");
        
        // Help/Info
        SameLine();
        Text("Help");
        
        // Right-aligned FPS counter
        SameLine();
        Text("|");
        SameLine();
        char fpsText[32];
        float fps = (m_frameTimeMs > 0.0f) ? (1000.0f / m_frameTimeMs) : 0.0f;
        snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps);
        TextColored(fps >= 58.0f ? Color::Green() : (fps >= 30.0f ? Color::Yellow() : Color::Red()), fpsText);
        
        m_uiContext->endWindow();
    }
}

void EngineOverlaySystem::toggleOverlay() {
    m_visible = !m_visible;
    std::cout << "Engine Overlay " << (m_visible ? "shown" : "hidden") << std::endl;
    if (m_consoleWindow) {
        m_consoleWindow->addLog(m_visible ? "Overlay shown (F1)" : "Overlay hidden (F1)", 
                               EngineUI::LogEntry::Level::Info);
    }
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
