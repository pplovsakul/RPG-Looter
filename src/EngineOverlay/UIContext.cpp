#include "UIContext.h"
#include <algorithm>
#include <cstring>

namespace EngineUI {

// ============================================================================
// InputState Implementation
// ============================================================================

void InputState::reset() {
    mouseDelta = glm::vec2(0, 0);
    mouseWheel = 0.0f;
    
    for (int i = 0; i < 5; ++i) {
        mouseClicked[i] = false;
        mouseReleased[i] = false;
    }
    
    for (int i = 0; i < 512; ++i) {
        keysPressed[i] = false;
        keysReleased[i] = false;
    }
    
    inputCharCount = 0;
    std::memset(inputCharacters, 0, sizeof(inputCharacters));
}

// ============================================================================
// UIContext Implementation
// ============================================================================

static UIContext* g_Context = nullptr;

UIContext* GetContext() {
    return g_Context;
}

void SetContext(UIContext* ctx) {
    g_Context = ctx;
}

UIContext* CreateContext() {
    UIContext* ctx = new UIContext();
    if (g_Context == nullptr) {
        SetContext(ctx);
    }
    return ctx;
}

void DestroyContext(UIContext* ctx) {
    if (g_Context == ctx) {
        SetContext(nullptr);
    }
    delete ctx;
}

UIContext::UIContext() {
}

UIContext::~UIContext() {
    shutdown();
}

bool UIContext::init(int screenWidth, int screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    
    if (!m_renderer.init(screenWidth, screenHeight)) {
        return false;
    }
    
    return true;
}

void UIContext::shutdown() {
    m_renderer.shutdown();
    m_windows.clear();
}

void UIContext::beginFrame(int screenWidth, int screenHeight) {
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    
    // Update input state
    updateInput();
    
    // Reset frame state
    m_hoveredID = 0;
    m_lastItemID = 0;
    m_wantCaptureMouse = false;
    m_wantCaptureKeyboard = false;
    
    // Clear window render order
    m_windowRenderOrder.clear();
    
    m_renderer.beginFrame(screenWidth, screenHeight);
}

void UIContext::endFrame() {
    // Determine what's hovered
    updateHover();
    
    // Store input for next frame
    m_inputPrev = m_input;
    m_input.reset();
    
    m_renderer.endFrame();
}

void UIContext::render() {
    // Render windows in order
    for (auto* window : m_windowRenderOrder) {
        if (window && window->open && !window->collapsed) {
            m_renderer.render(window->drawList);
        }
    }
}

void UIContext::updateInput() {
    // Calculate mouse delta
    m_input.mouseDelta = m_input.mousePos - m_inputPrev.mousePos;
    
    // Calculate clicked and released states
    for (int i = 0; i < 5; ++i) {
        m_input.mouseClicked[i] = m_input.mouseDown[i] && !m_inputPrev.mouseDown[i];
        m_input.mouseReleased[i] = !m_input.mouseDown[i] && m_inputPrev.mouseDown[i];
    }
    
    // Calculate key pressed and released states
    for (int i = 0; i < 512; ++i) {
        m_input.keysPressed[i] = m_input.keysDown[i] && !m_inputPrev.keysDown[i];
        m_input.keysReleased[i] = !m_input.keysDown[i] && m_inputPrev.keysDown[i];
    }
}

void UIContext::updateHover() {
    // Find which window is hovered (top-most)
    m_wantCaptureMouse = false;
    
    for (auto it = m_windowRenderOrder.rbegin(); it != m_windowRenderOrder.rend(); ++it) {
        WindowState* window = *it;
        if (!window || !window->open) continue;
        
        if (window->rect.contains(m_input.mousePos)) {
            window->hovered = true;
            m_wantCaptureMouse = true;
            
            // If clicked, bring to front and focus
            if (m_input.mouseClicked[0]) {
                window->focused = true;
                if (m_focusedWindow && m_focusedWindow != window) {
                    m_focusedWindow->focused = false;
                }
                m_focusedWindow = window;
            }
            break; // Only topmost window
        } else {
            window->hovered = false;
        }
    }
    
    // Check if any widget wants keyboard input
    if (m_activeID != 0) {
        m_wantCaptureKeyboard = true;
    }
}

bool UIContext::beginWindow(const char* name, bool* p_open, const Rect* initialRect) {
    // Get or create window state
    auto it = m_windows.find(name);
    if (it == m_windows.end()) {
        WindowState newWindow;
        newWindow.name = name;
        
        if (initialRect) {
            newWindow.rect = *initialRect;
        } else {
            // Default position and size
            newWindow.rect = Rect(100, 100, 400, 300);
        }
        
        newWindow.open = true;
        newWindow.appearing = true;
        
        it = m_windows.insert({name, newWindow}).first;
    }
    
    WindowState& window = it->second;
    m_currentWindow = &window;
    
    // Check if window should be closed
    if (p_open && !*p_open) {
        window.open = false;
    }
    
    if (!window.open) {
        m_currentWindow = nullptr;
        return false;
    }
    
    // Add to render order
    m_windowRenderOrder.push_back(&window);
    
    // Clear draw list
    window.drawList.clear();
    
    // Draw window background
    window.drawList.addRectFilled(window.rect, m_style.windowBg);
    window.drawList.addRect(window.rect, m_style.border, 0.0f, 1.0f);
    
    // Draw title bar
    Rect titleRect(window.rect.x, window.rect.y, window.rect.w, m_style.fontSize + m_style.windowPadding);
    Color titleColor = window.focused ? m_style.titleBgActive : m_style.titleBg;
    window.drawList.addRectFilled(titleRect, titleColor);
    
    // Setup content area
    window.contentRect = Rect(
        window.rect.x + m_style.windowPadding,
        window.rect.y + titleRect.h + m_style.windowPadding,
        window.rect.w - 2 * m_style.windowPadding,
        window.rect.h - titleRect.h - 2 * m_style.windowPadding
    );
    
    // Setup layout
    window.layout.cursor = glm::vec2(window.contentRect.x, window.contentRect.y);
    window.layout.cursorMax = window.layout.cursor;
    window.layout.contentWidth = window.contentRect.w;
    window.layout.lineHeight = m_style.lineHeight;
    window.layout.sameLine = false;
    
    // Push clip rect
    window.drawList.pushClipRect(window.contentRect);
    
    // Push window ID onto stack
    m_idStack.clear();
    m_idStack.push_back(hashID(name));
    
    return true;
}

void UIContext::endWindow() {
    if (!m_currentWindow) return;
    
    // Pop clip rect
    m_currentWindow->drawList.popClipRect();
    
    // Update scroll max
    m_currentWindow->scrollMax = glm::vec2(
        std::max(0.0f, m_currentWindow->layout.cursorMax.x - m_currentWindow->contentRect.x - m_currentWindow->contentRect.w),
        std::max(0.0f, m_currentWindow->layout.cursorMax.y - m_currentWindow->contentRect.y - m_currentWindow->contentRect.h)
    );
    
    m_currentWindow->appearing = false;
    m_currentWindow = nullptr;
}

WidgetID UIContext::getID(const char* str) {
    if (m_idStack.empty()) {
        return hashID(str);
    }
    
    // Combine with parent ID
    WidgetID parentID = m_idStack.back();
    WidgetID childID = hashID(str);
    return parentID ^ childID;
}

WidgetID UIContext::getID(const void* ptr) {
    if (m_idStack.empty()) {
        return hashID(ptr);
    }
    
    WidgetID parentID = m_idStack.back();
    WidgetID childID = hashID(ptr);
    return parentID ^ childID;
}

bool UIContext::isHovered(const Rect& rect) const {
    if (!m_currentWindow || !m_currentWindow->hovered) {
        return false;
    }
    
    return rect.contains(m_input.mousePos);
}

Rect UIContext::allocRect(float width, float height) {
    if (!m_currentWindow) {
        return Rect(0, 0, 0, 0);
    }
    
    LayoutState& layout = m_currentWindow->layout;
    
    if (layout.sameLine) {
        layout.cursor.x = layout.sameLineX;
        layout.sameLine = false;
    }
    
    // Use full width if width is 0 or negative
    if (width <= 0) {
        width = layout.contentWidth;
    }
    
    Rect rect(layout.cursor.x, layout.cursor.y, width, height);
    
    // Advance cursor
    layout.cursor.y += height + m_style.itemSpacing;
    
    // Update max
    layout.cursorMax.x = std::max(layout.cursorMax.x, layout.cursor.x + width);
    layout.cursorMax.y = std::max(layout.cursorMax.y, layout.cursor.y);
    
    m_lastItemRect = rect;
    
    return rect;
}

void UIContext::sameLine(float offset) {
    if (!m_currentWindow) return;
    
    LayoutState& layout = m_currentWindow->layout;
    layout.sameLine = true;
    layout.sameLineX = m_lastItemRect.right() + offset + m_style.itemSpacing;
}

void UIContext::newLine() {
    if (!m_currentWindow) return;
    
    LayoutState& layout = m_currentWindow->layout;
    layout.sameLine = false;
}

void UIContext::indent(float amount) {
    if (!m_currentWindow) return;
    
    if (amount == 0.0f) {
        amount = m_style.indentSpacing;
    }
    
    m_currentWindow->layout.cursor.x += amount;
    m_currentWindow->layout.contentWidth -= amount;
}

void UIContext::unindent(float amount) {
    if (!m_currentWindow) return;
    
    if (amount == 0.0f) {
        amount = m_style.indentSpacing;
    }
    
    m_currentWindow->layout.cursor.x -= amount;
    m_currentWindow->layout.contentWidth += amount;
}

DrawList& UIContext::getDrawList() {
    if (m_currentWindow) {
        return m_currentWindow->drawList;
    }
    
    // Return a dummy draw list (should not happen)
    static DrawList dummy;
    return dummy;
}

void UIContext::setMousePos(float x, float y) {
    m_input.mousePos = glm::vec2(x, y);
}

void UIContext::setMouseButton(int button, bool down) {
    if (button >= 0 && button < 5) {
        m_input.mouseDown[button] = down;
    }
}

void UIContext::setMouseWheel(float delta) {
    m_input.mouseWheel = delta;
}

void UIContext::setKeyState(int key, bool down) {
    if (key >= 0 && key < 512) {
        m_input.keysDown[key] = down;
    }
}

void UIContext::addInputCharacter(char c) {
    if (m_input.inputCharCount < 31) {
        m_input.inputCharacters[m_input.inputCharCount++] = c;
    }
}

bool UIContext::isMouseDown(int button) const {
    if (button >= 0 && button < 5) {
        return m_input.mouseDown[button];
    }
    return false;
}

bool UIContext::isMouseClicked(int button) const {
    if (button >= 0 && button < 5) {
        return m_input.mouseClicked[button];
    }
    return false;
}

bool UIContext::isMouseReleased(int button) const {
    if (button >= 0 && button < 5) {
        return m_input.mouseReleased[button];
    }
    return false;
}

bool UIContext::isItemClicked(int button) const {
    return m_lastItemID == m_hoveredID && isMouseClicked(button);
}

} // namespace EngineUI
