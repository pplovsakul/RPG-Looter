#pragma once

#include "DrawList.h"
#include "UIRenderer.h"
#include <string>
#include <unordered_map>
#include <stack>
#include <vector>
#include <functional>

namespace EngineUI {

// ============================================================================
// Input State
// ============================================================================

struct InputState {
    glm::vec2 mousePos;
    glm::vec2 mouseDelta;
    bool mouseDown[5] = {false};
    bool mouseClicked[5] = {false};
    bool mouseReleased[5] = {false};
    float mouseWheel = 0.0f;
    
    bool keysDown[512] = {false};
    bool keysPressed[512] = {false};
    bool keysReleased[512] = {false};
    
    char inputCharacters[32] = {0};
    int inputCharCount = 0;
    
    void reset();
};

// ============================================================================
// Widget ID
// ============================================================================

using WidgetID = uint32_t;

inline WidgetID hashID(const char* str) {
    uint32_t hash = 2166136261u;
    while (*str) {
        hash ^= (uint32_t)*str++;
        hash *= 16777619u;
    }
    return hash;
}

inline WidgetID hashID(const void* ptr) {
    uint64_t val = (uint64_t)ptr;
    return (uint32_t)(val ^ (val >> 32));
}

// ============================================================================
// Style
// ============================================================================

struct Style {
    // Colors
    Color windowBg = Color(0.1f, 0.1f, 0.1f, 0.95f);
    Color titleBg = Color(0.04f, 0.04f, 0.04f, 1.0f);
    Color titleBgActive = Color(0.16f, 0.29f, 0.48f, 1.0f);
    Color border = Color(0.43f, 0.43f, 0.50f, 0.50f);
    
    Color button = Color(0.26f, 0.59f, 0.98f, 0.40f);
    Color buttonHovered = Color(0.26f, 0.59f, 0.98f, 1.00f);
    Color buttonActive = Color(0.06f, 0.53f, 0.98f, 1.00f);
    
    Color text = Color(1.00f, 1.00f, 1.00f, 1.00f);
    Color textDisabled = Color(0.50f, 0.50f, 0.50f, 1.00f);
    
    Color frameBg = Color(0.43f, 0.43f, 0.50f, 0.50f);
    Color frameBgHovered = Color(0.26f, 0.59f, 0.98f, 0.40f);
    Color frameBgActive = Color(0.26f, 0.59f, 0.98f, 0.67f);
    
    Color checkMark = Color(0.26f, 0.59f, 0.98f, 1.00f);
    
    // Spacing
    float windowPadding = 8.0f;
    float itemSpacing = 4.0f;
    float itemInnerSpacing = 4.0f;
    float indentSpacing = 21.0f;
    
    // Sizing
    float scrollbarSize = 14.0f;
    float grabMinSize = 10.0f;
    
    // Rounding
    float windowRounding = 0.0f;
    float frameRounding = 0.0f;
    
    // Font
    float fontSize = 13.0f;
    float lineHeight = 16.0f;
};

// ============================================================================
// Layout State
// ============================================================================

struct LayoutState {
    glm::vec2 cursor;
    glm::vec2 cursorMax;
    float contentWidth;
    float lineHeight;
    bool sameLine = false;
    float sameLineX;
};

// ============================================================================
// Window State
// ============================================================================

struct WindowState {
    std::string name;
    Rect rect;
    Rect contentRect;
    glm::vec2 scroll;
    glm::vec2 scrollMax;
    bool open = true;
    bool collapsed = false;
    bool focused = false;
    bool hovered = false;
    bool appearing = false;
    
    LayoutState layout;
    DrawList drawList;
    
    // Docking
    bool docked = false;
    int dockID = -1;
};

// ============================================================================
// UI Context
// ============================================================================

class UIContext {
public:
    UIContext();
    ~UIContext();
    
    bool init(int screenWidth, int screenHeight);
    void shutdown();
    
    // Frame management
    void beginFrame(int screenWidth, int screenHeight);
    void endFrame();
    void render();
    
    // Input
    void setMousePos(float x, float y);
    void setMouseButton(int button, bool down);
    void setMouseWheel(float delta);
    void setKeyState(int key, bool down);
    void addInputCharacter(char c);
    
    // Style
    Style& getStyle() { return m_style; }
    
    // Window management
    bool beginWindow(const char* name, bool* p_open = nullptr, const Rect* initialRect = nullptr);
    void endWindow();
    
    WindowState* getCurrentWindow() { return m_currentWindow; }
    
    // Widget state
    bool isItemHovered() const { return m_hoveredID == m_lastItemID; }
    bool isItemActive() const { return m_activeID == m_lastItemID; }
    bool isItemClicked(int button = 0) const;
    
    // Internal widget helpers
    WidgetID getID(const char* str);
    WidgetID getID(const void* ptr);
    bool isHovered(const Rect& rect) const;
    bool isActive(WidgetID id) const { return m_activeID == id; }
    void setActive(WidgetID id) { m_activeID = id; }
    void clearActive() { m_activeID = 0; }
    void setLastItemID(WidgetID id) { m_lastItemID = id; }
    
    // Layout helpers
    Rect allocRect(float width, float height);
    void sameLine(float offset = 0.0f);
    void newLine();
    void indent(float amount = 0.0f);
    void unindent(float amount = 0.0f);
    
    // Drawing
    DrawList& getDrawList();
    
    // Input queries
    const InputState& getInput() const { return m_input; }
    bool isMouseDown(int button = 0) const;
    bool isMouseClicked(int button = 0) const;
    bool isMouseReleased(int button = 0) const;
    glm::vec2 getMousePos() const { return m_input.mousePos; }
    
private:
    int m_screenWidth = 0;
    int m_screenHeight = 0;
    
    Style m_style;
    InputState m_input;
    InputState m_inputPrev;
    UIRenderer m_renderer;
    
    // Window management
    std::unordered_map<std::string, WindowState> m_windows;
    WindowState* m_currentWindow = nullptr;
    std::vector<WindowState*> m_windowRenderOrder;
    WindowState* m_focusedWindow = nullptr;
    
    // Widget state
    WidgetID m_hoveredID = 0;
    WidgetID m_activeID = 0;
    WidgetID m_lastItemID = 0;
    Rect m_lastItemRect;
    
    // ID stack
    std::vector<WidgetID> m_idStack;
    
    // Capture flags
    bool m_wantCaptureMouse = false;
    bool m_wantCaptureKeyboard = false;
    
    void updateInput();
    void updateHover();
};

// Global context
UIContext* GetContext();
void SetContext(UIContext* ctx);
UIContext* CreateContext();
void DestroyContext(UIContext* ctx);

} // namespace EngineUI
