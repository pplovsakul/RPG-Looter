#pragma once

#include "GUITypes.h"
#include "GUIStyle.h"
#include "GUIDrawList.h"
#include <vector>
#include <string>
#include <stack>
#include <unordered_map>

namespace CustomGUI {

// Input state
struct GUIInputState {
    Vec2 mousePos;
    Vec2 mousePosPrev;
    Vec2 mouseDelta;
    bool mouseDown[GUIMouseButton_COUNT];
    bool mouseClicked[GUIMouseButton_COUNT];
    bool mouseReleased[GUIMouseButton_COUNT];
    bool mouseDoubleClicked[GUIMouseButton_COUNT];
    float mouseWheel;
    
    bool keysDown[GUIKey_COUNT];
    bool keysPressed[GUIKey_COUNT];
    bool keysReleased[GUIKey_COUNT];
    
    bool keyCtrl;
    bool keyShift;
    bool keyAlt;
    bool keySuper;
    
    std::string inputCharacters;
    
    GUIInputState();
    void clear();
};

// Window data
struct GUIWindow {
    std::string name;
    ID id;
    int flags;
    
    Vec2 pos;
    Vec2 size;
    Vec2 sizeFull;
    Vec2 contentSize;
    Vec2 scroll;
    Vec2 scrollTarget;
    
    bool active;
    bool wasActive;
    bool collapsed;
    bool appearing;
    bool hidden;
    
    float titleBarHeight;
    float menuBarHeight;
    
    Rect rect;
    Rect contentRect;
    
    GUIDrawList* drawList;
    
    GUIWindow();
};

// Item/Widget state
struct GUIItemState {
    ID id;
    Rect rect;
    bool hovered;
    bool active;
    bool clicked;
    
    GUIItemState() : id(0), hovered(false), active(false), clicked(false) {}
};

// Layout cursor
struct GUILayoutCursor {
    Vec2 pos;
    Vec2 lineStartPos;
    float lineHeight;
    float prevLineHeight;
    float indentX;
    
    GUILayoutCursor() : pos(0, 0), lineStartPos(0, 0), lineHeight(0), prevLineHeight(0), indentX(0) {}
};

// Main GUI Context
class GUIContext {
private:
    // Style and rendering
    GUIStyle style;
    GUIDrawList drawList;
    
    // Windows
    std::vector<GUIWindow*> windows;
    std::unordered_map<ID, GUIWindow*> windowsById;
    GUIWindow* currentWindow;
    std::stack<GUIWindow*> windowStack;
    
    // Input state
    GUIInputState input;
    float deltaTime;
    double time;
    int frameCount;
    
    // Widget state
    ID hoveredId;
    ID activeId;
    ID activeIdPreviousFrame;
    ID focusedId;
    
    // Layout
    GUILayoutCursor cursor;
    std::stack<Rect> clipRectStack;
    
    // Display size
    Vec2 displaySize;
    Vec2 displayFramebufferScale;
    
    // ID stack for generating unique IDs
    std::stack<ID> idStack;
    
    // Item state
    GUIItemState lastItemState;
    
    // Frame state
    bool frameStarted;
    
public:
    GUIContext();
    ~GUIContext();
    
    // Frame management
    void newFrame();
    void endFrame();
    void render();
    
    // Input management
    void setMousePos(float x, float y);
    void setMouseButton(int button, bool down);
    void setMouseWheel(float wheel);
    void setKeyDown(GUIKey key, bool down);
    void addInputCharacter(unsigned int c);
    void setKeyModifiers(bool ctrl, bool shift, bool alt, bool super);
    
    // Display management
    void setDisplaySize(float width, float height);
    Vec2 getDisplaySize() const { return displaySize; }
    
    // Window management
    bool begin(const char* name, bool* pOpen = nullptr, int flags = GUIWindowFlags_None);
    void end();
    
    // Style access
    GUIStyle& getStyle() { return style; }
    const GUIStyle& getStyle() const { return style; }
    
    // Drawing
    GUIDrawList& getDrawList() { return drawList; }
    const GUIDrawList& getDrawList() const { return drawList; }
    GUIDrawList* getWindowDrawList();
    
    // ID generation
    ID getID(const char* str);
    ID getID(const void* ptr);
    void pushID(const char* str);
    void pushID(const void* ptr);
    void pushID(int id);
    void popID();
    
    // Layout
    void sameLine(float offsetFromStartX = 0.0f, float spacing = -1.0f);
    void newLine();
    void spacing();
    void dummy(const Vec2& size);
    void indent(float width = 0.0f);
    void unindent(float width = 0.0f);
    void separator();
    
    Vec2 getCursorPos() const;
    void setCursorPos(const Vec2& pos);
    Vec2 getCursorScreenPos() const;
    void setCursorScreenPos(const Vec2& pos);
    
    // Widget helpers
    bool isItemHovered() const { return lastItemState.hovered; }
    bool isItemActive() const { return lastItemState.active; }
    bool isItemClicked(int mouseButton = 0) const { return lastItemState.clicked; }
    Rect getItemRect() const { return lastItemState.rect; }
    
    // Window queries
    Vec2 getWindowPos() const;
    Vec2 getWindowSize() const;
    Vec2 getContentRegionAvail() const;
    
    // Input queries
    Vec2 getMousePos() const { return input.mousePos; }
    bool isMouseDown(int button) const { 
        return (button >= 0 && button < GUIMouseButton_COUNT) ? input.mouseDown[button] : false; 
    }
    
    // Collision detection
    bool isMouseHoveringRect(const Vec2& min, const Vec2& max) const;
    
    // Widget behavior helpers (for custom widgets)
    bool buttonBehavior(const Rect& bb, ID id, bool* outHovered, bool* outHeld);
    
private:
    GUIWindow* findWindowByName(const char* name);
    GUIWindow* createWindow(const char* name, const Vec2& pos, const Vec2& size, int flags);
    void destroyWindow(GUIWindow* window);
    
    void updateWindowLayout(GUIWindow* window);
    void renderWindow(GUIWindow* window);
    
    void itemSize(const Vec2& size, float textOffsetY = 0.0f);
    bool itemAdd(const Rect& bb, ID id);
    
    Vec2 calcItemSize(const Vec2& size, float defaultWidth, float defaultHeight);
    
    ID generateIDFromString(const char* str);
    ID generateIDFromPointer(const void* ptr);
    
    void clearActiveID();
    void setActiveID(ID id);
    void setHoveredID(ID id);
};

// Global context access
GUIContext* createContext();
void destroyContext(GUIContext* ctx);
void setCurrentContext(GUIContext* ctx);
GUIContext* getCurrentContext();

} // namespace CustomGUI
