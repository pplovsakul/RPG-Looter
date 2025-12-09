#include "GUIContext.h"
#include <algorithm>
#include <cstring>
#include <functional>

namespace CustomGUI {

// Global context pointer
static GUIContext* g_CurrentContext = nullptr;

// Hash function for ID generation
static ID hashString(const char* str) {
    ID hash = 0x811c9dc5;
    while (*str) {
        hash = ((hash ^ static_cast<unsigned char>(*str++)) * 0x01000193);
    }
    return hash;
}

//-----------------------------------------------------------------------------
// GUIInputState
//-----------------------------------------------------------------------------

GUIInputState::GUIInputState() {
    clear();
}

void GUIInputState::clear() {
    mousePos = Vec2(0, 0);
    mousePosPrev = Vec2(0, 0);
    mouseDelta = Vec2(0, 0);
    mouseWheel = 0.0f;
    
    for (int i = 0; i < GUIMouseButton_COUNT; i++) {
        mouseDown[i] = false;
        mouseClicked[i] = false;
        mouseReleased[i] = false;
        mouseDoubleClicked[i] = false;
    }
    
    for (int i = 0; i < GUIKey_COUNT; i++) {
        keysDown[i] = false;
        keysPressed[i] = false;
        keysReleased[i] = false;
    }
    
    keyCtrl = false;
    keyShift = false;
    keyAlt = false;
    keySuper = false;
    
    inputCharacters.clear();
}

//-----------------------------------------------------------------------------
// GUIWindow
//-----------------------------------------------------------------------------

GUIWindow::GUIWindow()
    : id(0), flags(0), pos(0, 0), size(100, 100), sizeFull(100, 100),
      contentSize(0, 0), scroll(0, 0), scrollTarget(0, 0),
      active(false), wasActive(false), collapsed(false), appearing(false), hidden(false),
      titleBarHeight(0), menuBarHeight(0), drawList(nullptr) {
}

//-----------------------------------------------------------------------------
// GUIContext
//-----------------------------------------------------------------------------

GUIContext::GUIContext()
    : currentWindow(nullptr), hoveredId(0), activeId(0), activeIdPreviousFrame(0), focusedId(0),
      deltaTime(0.0f), time(0.0), frameCount(0), displaySize(800, 600), 
      displayFramebufferScale(1, 1), frameStarted(false) {
}

GUIContext::~GUIContext() {
    for (auto* window : windows) {
        delete window;
    }
}

void GUIContext::newFrame() {
    frameCount++;
    frameStarted = true;
    
    // Update input state
    input.mouseDelta = input.mousePos - input.mousePosPrev;
    input.mousePosPrev = input.mousePos;
    
    // Clear clicked states
    for (int i = 0; i < GUIMouseButton_COUNT; i++) {
        input.mouseClicked[i] = false;
        input.mouseReleased[i] = false;
        input.mouseDoubleClicked[i] = false;
    }
    
    for (int i = 0; i < GUIKey_COUNT; i++) {
        input.keysPressed[i] = false;
        input.keysReleased[i] = false;
    }
    
    // Clear draw list
    drawList.clear();
    
    // Update active ID
    activeIdPreviousFrame = activeId;
    hoveredId = 0;
}

void GUIContext::endFrame() {
    frameStarted = false;
    
    // Clear input characters for next frame
    input.inputCharacters.clear();
}

void GUIContext::render() {
    // Render all windows
    for (auto* window : windows) {
        if (window->active && !window->hidden) {
            renderWindow(window);
        }
    }
}

void GUIContext::setMousePos(float x, float y) {
    input.mousePos = Vec2(x, y);
}

void GUIContext::setMouseButton(int button, bool down) {
    if (button >= 0 && button < GUIMouseButton_COUNT) {
        bool wasDown = input.mouseDown[button];
        input.mouseDown[button] = down;
        
        if (down && !wasDown) {
            input.mouseClicked[button] = true;
        }
        if (!down && wasDown) {
            input.mouseReleased[button] = true;
        }
    }
}

void GUIContext::setMouseWheel(float wheel) {
    input.mouseWheel = wheel;
}

void GUIContext::setKeyDown(GUIKey key, bool down) {
    if (key >= 0 && key < GUIKey_COUNT) {
        bool wasDown = input.keysDown[key];
        input.keysDown[key] = down;
        
        if (down && !wasDown) {
            input.keysPressed[key] = true;
        }
        if (!down && wasDown) {
            input.keysReleased[key] = true;
        }
    }
}

void GUIContext::addInputCharacter(unsigned int c) {
    // Note: This is a simplified ASCII-only implementation.
    // For full Unicode support, this should be converted to UTF-8 encoding.
    // Currently only supports ASCII characters (0-127).
    if (c > 0 && c < 128) {
        input.inputCharacters += static_cast<char>(c);
    }
}

void GUIContext::setKeyModifiers(bool ctrl, bool shift, bool alt, bool super) {
    input.keyCtrl = ctrl;
    input.keyShift = shift;
    input.keyAlt = alt;
    input.keySuper = super;
}

void GUIContext::setDisplaySize(float width, float height) {
    displaySize = Vec2(width, height);
}

bool GUIContext::begin(const char* name, bool* pOpen, int flags) {
    GUIWindow* window = findWindowByName(name);
    
    if (!window) {
        Vec2 pos(50, 50);
        Vec2 size(400, 300);
        window = createWindow(name, pos, size, flags);
    }
    
    window->flags = flags;
    window->wasActive = window->active;
    window->active = true;
    
    if (pOpen && !*pOpen) {
        window->active = false;
        window->hidden = true;
        return false;
    }
    
    window->hidden = false;
    
    // Push window to stack
    currentWindow = window;
    windowStack.push(window);
    
    // Initialize window layout
    updateWindowLayout(window);
    
    // Setup cursor position
    cursor.pos = window->pos + style.WindowPadding;
    cursor.lineStartPos = cursor.pos;
    cursor.lineHeight = 0;
    cursor.prevLineHeight = 0;
    cursor.indentX = 0;
    
    if (!(flags & GUIWindowFlags_NoTitleBar)) {
        cursor.pos.y += window->titleBarHeight;
    }
    
    return !window->collapsed;
}

void GUIContext::end() {
    if (!windowStack.empty()) {
        windowStack.pop();
    }
    
    if (!windowStack.empty()) {
        currentWindow = windowStack.top();
    } else {
        currentWindow = nullptr;
    }
}

GUIDrawList* GUIContext::getWindowDrawList() {
    if (currentWindow && currentWindow->drawList) {
        return currentWindow->drawList;
    }
    return &drawList;
}

ID GUIContext::getID(const char* str) {
    return generateIDFromString(str);
}

ID GUIContext::getID(const void* ptr) {
    return generateIDFromPointer(ptr);
}

void GUIContext::pushID(const char* str) {
    idStack.push(generateIDFromString(str));
}

void GUIContext::pushID(const void* ptr) {
    idStack.push(generateIDFromPointer(ptr));
}

void GUIContext::pushID(int id) {
    idStack.push(static_cast<ID>(id));
}

void GUIContext::popID() {
    if (!idStack.empty()) {
        idStack.pop();
    }
}

void GUIContext::sameLine(float offsetFromStartX, float spacing) {
    if (spacing < 0.0f)
        spacing = style.ItemSpacing.x;
    
    cursor.pos.x = cursor.lineStartPos.x + offsetFromStartX + spacing;
    cursor.pos.y = cursor.lineStartPos.y;
}

void GUIContext::newLine() {
    cursor.pos.x = cursor.lineStartPos.x;
    cursor.pos.y += cursor.lineHeight + style.ItemSpacing.y;
    cursor.lineHeight = 0;
}

void GUIContext::spacing() {
    cursor.pos.y += style.ItemSpacing.y;
}

void GUIContext::dummy(const Vec2& size) {
    itemSize(size);
}

void GUIContext::indent(float width) {
    if (width == 0.0f)
        width = style.IndentSpacing;
    cursor.indentX += width;
    cursor.pos.x += width;
}

void GUIContext::unindent(float width) {
    if (width == 0.0f)
        width = style.IndentSpacing;
    cursor.indentX -= width;
    cursor.pos.x -= width;
}

void GUIContext::separator() {
    if (!currentWindow)
        return;
    
    Vec2 p1 = cursor.pos;
    Vec2 p2 = Vec2(currentWindow->pos.x + currentWindow->size.x - style.WindowPadding.x, cursor.pos.y);
    
    drawList.addLine(p1, p2, style.getColor(GUICol_Separator), 1.0f);
    
    cursor.pos.y += 1.0f + style.ItemSpacing.y;
}

Vec2 GUIContext::getCursorPos() const {
    if (!currentWindow)
        return Vec2(0, 0);
    return cursor.pos - currentWindow->pos;
}

void GUIContext::setCursorPos(const Vec2& pos) {
    if (!currentWindow)
        return;
    cursor.pos = currentWindow->pos + pos;
}

Vec2 GUIContext::getCursorScreenPos() const {
    return cursor.pos;
}

void GUIContext::setCursorScreenPos(const Vec2& pos) {
    cursor.pos = pos;
}

Vec2 GUIContext::getWindowPos() const {
    if (!currentWindow)
        return Vec2(0, 0);
    return currentWindow->pos;
}

Vec2 GUIContext::getWindowSize() const {
    if (!currentWindow)
        return Vec2(0, 0);
    return currentWindow->size;
}

Vec2 GUIContext::getContentRegionAvail() const {
    if (!currentWindow)
        return displaySize;
    
    Vec2 max = currentWindow->pos + currentWindow->size - style.WindowPadding;
    Vec2 avail = max - cursor.pos;
    return Vec2(std::max(0.0f, avail.x), std::max(0.0f, avail.y));
}

bool GUIContext::isMouseHoveringRect(const Vec2& min, const Vec2& max) const {
    return input.mousePos.x >= min.x && input.mousePos.y >= min.y &&
           input.mousePos.x < max.x && input.mousePos.y < max.y;
}

GUIWindow* GUIContext::findWindowByName(const char* name) {
    ID id = hashString(name);
    auto it = windowsById.find(id);
    if (it != windowsById.end()) {
        return it->second;
    }
    return nullptr;
}

GUIWindow* GUIContext::createWindow(const char* name, const Vec2& pos, const Vec2& size, int flags) {
    GUIWindow* window = new GUIWindow();
    window->name = name;
    window->id = hashString(name);
    window->pos = pos;
    window->size = size;
    window->sizeFull = size;
    window->flags = flags;
    window->titleBarHeight = (flags & GUIWindowFlags_NoTitleBar) ? 0.0f : 25.0f;
    window->drawList = new GUIDrawList();
    
    windows.push_back(window);
    windowsById[window->id] = window;
    
    return window;
}

void GUIContext::destroyWindow(GUIWindow* window) {
    if (!window)
        return;
    
    windowsById.erase(window->id);
    windows.erase(std::remove(windows.begin(), windows.end(), window), windows.end());
    delete window->drawList;
    delete window;
}

void GUIContext::updateWindowLayout(GUIWindow* window) {
    window->rect = Rect(window->pos, window->pos + window->size);
    window->contentRect = window->rect;
    window->contentRect.min += style.WindowPadding;
    window->contentRect.max -= style.WindowPadding;
}

void GUIContext::renderWindow(GUIWindow* window) {
    if (!window || window->hidden)
        return;
    
    GUIDrawList* dl = window->drawList ? window->drawList : &drawList;
    
    // Draw window background
    if (!(window->flags & GUIWindowFlags_NoBackground)) {
        dl->addRectFilled(window->rect.min, window->rect.max, 
                         style.getColor(GUICol_WindowBg), style.WindowRounding);
    }
    
    // Draw window border
    if (window->flags & GUIWindowFlags_None || !(window->flags & GUIWindowFlags_NoTitleBar)) {
        dl->addRect(window->rect.min, window->rect.max, 
                   style.getColor(GUICol_Border), style.WindowRounding, style.WindowBorderSize);
    }
    
    // Draw title bar
    if (!(window->flags & GUIWindowFlags_NoTitleBar)) {
        Rect titleRect(window->pos, Vec2(window->pos.x + window->size.x, window->pos.y + window->titleBarHeight));
        dl->addRectFilled(titleRect.min, titleRect.max, style.getColor(GUICol_TitleBgActive), 0.0f);
        
        // Draw title text (simplified - would use proper font rendering)
        Vec2 textPos = titleRect.min + Vec2(style.WindowPadding.x, (titleRect.getHeight() - 14.0f) * 0.5f);
        dl->addText(textPos, style.getColor(GUICol_Text), window->name.c_str());
    }
    
    // Copy window draw list to main draw list
    if (window->drawList) {
        const auto& cmds = window->drawList->getCommands();
        const auto& vtx = window->drawList->getVertices();
        const auto& idx = window->drawList->getIndices();
        
        // Merge into main draw list (simplified)
        for (const auto& cmd : cmds) {
            drawList.getCommands().push_back(cmd);
        }
    }
}

bool GUIContext::buttonBehavior(const Rect& bb, ID id, bool* outHovered, bool* outHeld) {
    bool hovered = isMouseHoveringRect(bb.min, bb.max);
    bool held = false;
    bool pressed = false;
    
    if (hovered) {
        setHoveredID(id);
        
        if (input.mouseClicked[0]) {
            setActiveID(id);
        }
    }
    
    if (activeId == id) {
        held = input.mouseDown[0];
        
        if (input.mouseReleased[0]) {
            if (hovered) {
                pressed = true;
            }
            clearActiveID();
        }
    }
    
    if (outHovered) *outHovered = hovered;
    if (outHeld) *outHeld = held;
    
    return pressed;
}

void GUIContext::itemSize(const Vec2& size, float textOffsetY) {
    cursor.lineHeight = std::max(cursor.lineHeight, size.y);
    cursor.pos.x += size.x;
}

bool GUIContext::itemAdd(const Rect& bb, ID id) {
    lastItemState.id = id;
    lastItemState.rect = bb;
    lastItemState.hovered = isMouseHoveringRect(bb.min, bb.max);
    lastItemState.active = (activeId == id);
    lastItemState.clicked = false;
    
    return true;
}

Vec2 GUIContext::calcItemSize(const Vec2& size, float defaultWidth, float defaultHeight) {
    Vec2 result = size;
    if (result.x <= 0.0f)
        result.x = defaultWidth;
    if (result.y <= 0.0f)
        result.y = defaultHeight;
    return result;
}

ID GUIContext::generateIDFromString(const char* str) {
    ID seed = idStack.empty() ? 0 : idStack.top();
    ID id = hashString(str);
    return seed ^ id;
}

ID GUIContext::generateIDFromPointer(const void* ptr) {
    ID seed = idStack.empty() ? 0 : idStack.top();
    ID id = static_cast<ID>(reinterpret_cast<uintptr_t>(ptr));
    return seed ^ id;
}

void GUIContext::clearActiveID() {
    activeId = 0;
}

void GUIContext::setActiveID(ID id) {
    activeId = id;
}

void GUIContext::setHoveredID(ID id) {
    hoveredId = id;
}

//-----------------------------------------------------------------------------
// Global context functions
//-----------------------------------------------------------------------------

GUIContext* createContext() {
    GUIContext* ctx = new GUIContext();
    if (!g_CurrentContext) {
        setCurrentContext(ctx);
    }
    return ctx;
}

void destroyContext(GUIContext* ctx) {
    if (g_CurrentContext == ctx) {
        setCurrentContext(nullptr);
    }
    delete ctx;
}

void setCurrentContext(GUIContext* ctx) {
    g_CurrentContext = ctx;
}

GUIContext* getCurrentContext() {
    return g_CurrentContext;
}

} // namespace CustomGUI
