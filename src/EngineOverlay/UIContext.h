#pragma once

#include "DrawList.h"
#include "UIRenderer.h"
#include "FontAtlas.h"
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
    // Colors - Yellow/Golden ImGui-inspired theme
    Color windowBg = Color(0.94f, 0.94f, 0.86f, 1.0f);  // Light beige background - fully opaque
    Color titleBg = Color(0.80f, 0.70f, 0.20f, 1.0f);     // Golden title bar
    Color titleBgActive = Color(1.00f, 0.85f, 0.00f, 1.0f); // Bright yellow when active
    Color border = Color(0.60f, 0.50f, 0.10f, 1.0f);     // Dark golden border - fully opaque
    
    Color button = Color(0.90f, 0.75f, 0.20f, 0.80f);     // Yellow button
    Color buttonHovered = Color(1.00f, 0.85f, 0.30f, 1.00f); // Bright yellow on hover
    Color buttonActive = Color(0.80f, 0.65f, 0.10f, 1.00f);  // Darker yellow when pressed
    
    Color text = Color(0.00f, 0.00f, 0.00f, 1.00f);       // Black text for contrast
    Color textDisabled = Color(0.40f, 0.40f, 0.40f, 1.00f); // Gray disabled text
    
    Color frameBg = Color(0.85f, 0.85f, 0.75f, 0.80f);    // Light frame background
    Color frameBgHovered = Color(0.95f, 0.85f, 0.50f, 0.80f); // Yellow-tinted on hover
    Color frameBgActive = Color(0.90f, 0.75f, 0.30f, 0.90f);  // More yellow when active
    
    Color checkMark = Color(0.80f, 0.65f, 0.00f, 1.00f);  // Dark yellow checkmark
    
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
    void drawText(const glm::vec2& pos, const Color& color, const char* text);
    glm::vec2 measureText(const char* text) const;
    
    // Input queries
    const InputState& getInput() const { return m_input; }
    bool isMouseDown(int button = 0) const;
    bool isMouseClicked(int button = 0) const;
    bool isMouseReleased(int button = 0) const;
    glm::vec2 getMousePos() const { return m_input.mousePos; }
    
    // Get current window's font atlas if available
    FontAtlas* getFontAtlas() { return &m_fontAtlas; }
    
private:
    int m_screenWidth = 0;
    int m_screenHeight = 0;
    
    Style m_style;
    InputState m_input;
    InputState m_inputPrev;
    UIRenderer m_renderer;
    FontAtlas m_fontAtlas;
    
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
