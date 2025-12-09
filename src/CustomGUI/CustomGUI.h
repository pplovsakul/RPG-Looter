#pragma once

// Custom Immediate-Mode GUI System
// Similar to ImGui but with custom implementation
// 
// Usage:
//   CustomGUI::createContext();
//   
//   // In your main loop:
//   CustomGUI::newFrame();
//   
//   if (CustomGUI::begin("My Window")) {
//       CustomGUI::text("Hello, World!");
//       if (CustomGUI::button("Click Me")) {
//           // Button was clicked
//       }
//       CustomGUI::sliderFloat("Value", &myFloat, 0.0f, 1.0f);
//   }
//   CustomGUI::end();
//   
//   CustomGUI::endFrame();
//   CustomGUI::render();

#include "GUIContext.h"
#include "GUIStyle.h"
#include "GUIDrawList.h"
#include "GUITypes.h"

namespace CustomGUI {

//-----------------------------------------------------------------------------
// Context Management
//-----------------------------------------------------------------------------

inline GUIContext* createContext() { return CustomGUI::createContext(); }
inline void destroyContext(GUIContext* ctx) { CustomGUI::destroyContext(ctx); }
inline void setCurrentContext(GUIContext* ctx) { CustomGUI::setCurrentContext(ctx); }
inline GUIContext* getCurrentContext() { return CustomGUI::getCurrentContext(); }

//-----------------------------------------------------------------------------
// Frame Management
//-----------------------------------------------------------------------------

inline void newFrame() {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->newFrame();
}

inline void endFrame() {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->endFrame();
}

inline void render() {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->render();
}

//-----------------------------------------------------------------------------
// Style
//-----------------------------------------------------------------------------

inline GUIStyle& getStyle() {
    GUIContext* ctx = getCurrentContext();
    return ctx->getStyle();
}

inline void styleColorsDark() {
    getStyle().setDarkTheme();
}

inline void styleColorsLight() {
    getStyle().setLightTheme();
}

inline void styleColorsClassic() {
    getStyle().setClassicTheme();
}

//-----------------------------------------------------------------------------
// Window Management
//-----------------------------------------------------------------------------

inline bool begin(const char* name, bool* pOpen = nullptr, int flags = GUIWindowFlags_None) {
    GUIContext* ctx = getCurrentContext();
    if (ctx) return ctx->begin(name, pOpen, flags);
    return false;
}

inline void end() {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->end();
}

//-----------------------------------------------------------------------------
// Input Management
//-----------------------------------------------------------------------------

inline void setDisplaySize(float width, float height) {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->setDisplaySize(width, height);
}

inline void setMousePos(float x, float y) {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->setMousePos(x, y);
}

inline void setMouseButton(int button, bool down) {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->setMouseButton(button, down);
}

//-----------------------------------------------------------------------------
// Layout
//-----------------------------------------------------------------------------

inline void sameLine(float offsetFromStartX = 0.0f, float spacing = -1.0f) {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->sameLine(offsetFromStartX, spacing);
}

inline void newLine() {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->newLine();
}

inline void spacing() {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->spacing();
}

inline void dummy(const Vec2& size) {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->dummy(size);
}

inline void indent(float width = 0.0f) {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->indent(width);
}

inline void unindent(float width = 0.0f) {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->unindent(width);
}

inline void separator() {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->separator();
}

inline Vec2 getCursorPos() {
    GUIContext* ctx = getCurrentContext();
    if (ctx) return ctx->getCursorPos();
    return Vec2(0, 0);
}

inline void setCursorPos(const Vec2& pos) {
    GUIContext* ctx = getCurrentContext();
    if (ctx) ctx->setCursorPos(pos);
}

//-----------------------------------------------------------------------------
// Widgets - implemented in CustomGUI.cpp
//-----------------------------------------------------------------------------

// Text
void text(const char* fmt, ...);
void textColored(const Color& col, const char* fmt, ...);
void textDisabled(const char* fmt, ...);
void textWrapped(const char* fmt, ...);
void labelText(const char* label, const char* fmt, ...);
void bulletText(const char* fmt, ...);

// Buttons
bool button(const char* label, const Vec2& size = Vec2(0, 0));
bool smallButton(const char* label);
bool invisibleButton(const char* id, const Vec2& size);

// Checkbox & Radio
bool checkbox(const char* label, bool* v);
bool checkboxFlags(const char* label, int* flags, int flagsValue);
bool radioButton(const char* label, bool active);
bool radioButton(const char* label, int* v, int vButton);

// Sliders
bool sliderFloat(const char* label, float* v, float vMin, float vMax, const char* format = "%.3f");
bool sliderFloat2(const char* label, float v[2], float vMin, float vMax, const char* format = "%.3f");
bool sliderFloat3(const char* label, float v[3], float vMin, float vMax, const char* format = "%.3f");
bool sliderFloat4(const char* label, float v[4], float vMin, float vMax, const char* format = "%.3f");
bool sliderInt(const char* label, int* v, int vMin, int vMax, const char* format = "%d");
bool sliderInt2(const char* label, int v[2], int vMin, int vMax, const char* format = "%d");
bool sliderInt3(const char* label, int v[3], int vMin, int vMax, const char* format = "%d");
bool sliderInt4(const char* label, int v[4], int vMin, int vMax, const char* format = "%d");

// Input with Keyboard
bool inputText(const char* label, char* buf, size_t bufSize);
bool inputTextMultiline(const char* label, char* buf, size_t bufSize, const Vec2& size = Vec2(0, 0));
bool inputFloat(const char* label, float* v, float step = 0.0f, float stepFast = 0.0f, const char* format = "%.3f");
bool inputInt(const char* label, int* v, int step = 1, int stepFast = 100);

// Color Editor/Picker
bool colorEdit3(const char* label, float col[3]);
bool colorEdit4(const char* label, float col[4]);
bool colorPicker3(const char* label, float col[3]);
bool colorPicker4(const char* label, float col[4]);

// Trees
bool treeNode(const char* label);
bool treeNode(const char* strId, const char* fmt, ...);
bool treeNodeEx(const char* label, int flags = 0);
void treePop();

// Selectables
bool selectable(const char* label, bool selected = false, int flags = 0, const Vec2& size = Vec2(0, 0));
bool selectable(const char* label, bool* pSelected, int flags = 0, const Vec2& size = Vec2(0, 0));

// Combos
bool beginCombo(const char* label, const char* previewValue, int flags = 0);
void endCombo();
bool combo(const char* label, int* currentItem, const char* const items[], int itemsCount, int popupMaxHeightInItems = -1);

// Menus
bool beginMenuBar();
void endMenuBar();
bool beginMenu(const char* label, bool enabled = true);
void endMenu();
bool menuItem(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true);
bool menuItem(const char* label, const char* shortcut, bool* pSelected, bool enabled = true);

// Tooltips
void beginTooltip();
void endTooltip();
void setTooltip(const char* fmt, ...);

// Popups
void openPopup(const char* strId);
bool beginPopup(const char* strId);
void endPopup();
void closeCurrentPopup();

// Item/Widget Queries
bool isItemHovered();
bool isItemActive();
bool isItemClicked(int mouseButton = 0);

//-----------------------------------------------------------------------------
// Drawing API
//-----------------------------------------------------------------------------

inline GUIDrawList* getWindowDrawList() {
    GUIContext* ctx = getCurrentContext();
    if (ctx) return ctx->getWindowDrawList();
    return nullptr;
}

inline GUIDrawList& getBackgroundDrawList() {
    GUIContext* ctx = getCurrentContext();
    return ctx->getDrawList();
}

} // namespace CustomGUI
