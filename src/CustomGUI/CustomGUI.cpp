#include "CustomGUI.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <algorithm>

namespace CustomGUI {

// Helper to format text
static void formatText(char* buffer, size_t bufferSize, const char* fmt, va_list args) {
    vsnprintf(buffer, bufferSize, fmt, args);
    buffer[bufferSize - 1] = '\0';
}

//-----------------------------------------------------------------------------
// Text Widgets
//-----------------------------------------------------------------------------

void text(const char* fmt, ...) {
    GUIContext* ctx = getCurrentContext();
    if (!ctx) return;
    
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    formatText(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    GUIDrawList* drawList = ctx->getWindowDrawList();
    if (!drawList) return;
    
    Vec2 pos = ctx->getCursorScreenPos();
    const GUIStyle& style = ctx->getStyle();
    
    drawList->addText(pos, style.getColor(GUICol_Text), buffer);
    
    // Approximate text size
    float textWidth = std::strlen(buffer) * 8.0f;
    float textHeight = 16.0f;
    
    Vec2 textSize(textWidth, textHeight);
    ctx->dummy(textSize);
    ctx->newLine();
}

void textColored(const Color& col, const char* fmt, ...) {
    GUIContext* ctx = getCurrentContext();
    if (!ctx) return;
    
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    formatText(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    GUIDrawList* drawList = ctx->getWindowDrawList();
    if (!drawList) return;
    
    Vec2 pos = ctx->getCursorScreenPos();
    drawList->addText(pos, col, buffer);
    
    float textWidth = std::strlen(buffer) * 8.0f;
    float textHeight = 16.0f;
    Vec2 textSize(textWidth, textHeight);
    ctx->dummy(textSize);
    ctx->newLine();
}

void textDisabled(const char* fmt, ...) {
    GUIContext* ctx = getCurrentContext();
    if (!ctx) return;
    
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    formatText(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    const GUIStyle& style = ctx->getStyle();
    textColored(style.getColor(GUICol_TextDisabled), "%s", buffer);
}

void textWrapped(const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    formatText(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    text("%s", buffer);
}

void labelText(const char* label, const char* fmt, ...) {
    GUIContext* ctx = getCurrentContext();
    if (!ctx) return;
    
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    formatText(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    text("%s: %s", label, buffer);
}

void bulletText(const char* fmt, ...) {
    GUIContext* ctx = getCurrentContext();
    if (!ctx) return;
    
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    formatText(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    text("• %s", buffer);
}

//-----------------------------------------------------------------------------
// Button Widgets
//-----------------------------------------------------------------------------

bool button(const char* label, const Vec2& size) {
    GUIContext* ctx = getCurrentContext();
    if (!ctx) return false;
    
    const GUIStyle& style = ctx->getStyle();
    Vec2 pos = ctx->getCursorScreenPos();
    
    // Calculate button size
    float labelWidth = std::strlen(label) * 8.0f;
    float labelHeight = 16.0f;
    
    Vec2 buttonSize = size;
    if (buttonSize.x <= 0.0f)
        buttonSize.x = labelWidth + style.FramePadding.x * 2.0f;
    if (buttonSize.y <= 0.0f)
        buttonSize.y = labelHeight + style.FramePadding.y * 2.0f;
    
    Rect bb(pos, pos + buttonSize);
    
    // Button behavior
    ID id = ctx->getID(label);
    bool hovered = false;
    bool held = false;
    bool pressed = ctx->buttonBehavior(bb, id, &hovered, &held);
    
    // Render button
    GUIDrawList* drawList = ctx->getWindowDrawList();
    if (drawList) {
        Color col = style.getColor(GUICol_Button);
        if (held)
            col = style.getColor(GUICol_ButtonActive);
        else if (hovered)
            col = style.getColor(GUICol_ButtonHovered);
        
        drawList->addRectFilled(bb.min, bb.max, col, style.FrameRounding);
        
        if (style.FrameBorderSize > 0.0f) {
            drawList->addRect(bb.min, bb.max, style.getColor(GUICol_Border), 
                            style.FrameRounding, style.FrameBorderSize);
        }
        
        // Draw text centered
        Vec2 textPos = bb.min + Vec2((buttonSize.x - labelWidth) * 0.5f, 
                                     (buttonSize.y - labelHeight) * 0.5f);
        drawList->addText(textPos, style.getColor(GUICol_Text), label);
    }
    
    ctx->dummy(buttonSize);
    ctx->newLine();
    
    return pressed;
}

bool smallButton(const char* label) {
    GUIContext* ctx = getCurrentContext();
    if (!ctx) return false;
    
    const GUIStyle& style = ctx->getStyle();
    float labelWidth = std::strlen(label) * 7.0f;
    float labelHeight = 14.0f;
    
    Vec2 size(labelWidth + style.FramePadding.x, labelHeight + style.FramePadding.y);
    return button(label, size);
}

bool invisibleButton(const char* id, const Vec2& size) {
    GUIContext* ctx = getCurrentContext();
    if (!ctx) return false;
    
    Vec2 pos = ctx->getCursorScreenPos();
    Rect bb(pos, pos + size);
    
    ID buttonId = ctx->getID(id);
    bool hovered = false;
    bool held = false;
    bool pressed = ctx->buttonBehavior(bb, buttonId, &hovered, &held);
    
    ctx->dummy(size);
    ctx->newLine();
    
    return pressed;
}

//-----------------------------------------------------------------------------
// Checkbox & Radio Widgets
//-----------------------------------------------------------------------------

bool checkbox(const char* label, bool* v) {
    GUIContext* ctx = getCurrentContext();
    if (!ctx || !v) return false;
    
    const GUIStyle& style = ctx->getStyle();
    Vec2 pos = ctx->getCursorScreenPos();
    
    float checkSize = 16.0f;
    float labelWidth = std::strlen(label) * 8.0f;
    
    Vec2 checkboxPos = pos;
    Vec2 checkboxSize(checkSize, checkSize);
    Rect bb(checkboxPos, checkboxPos + checkboxSize);
    
    ID id = ctx->getID(label);
    bool hovered = false;
    bool held = false;
    bool pressed = ctx->buttonBehavior(bb, id, &hovered, &held);
    
    if (pressed) {
        *v = !*v;
    }
    
    // Render checkbox
    GUIDrawList* drawList = ctx->getWindowDrawList();
    if (drawList) {
        Color col = style.getColor(GUICol_FrameBg);
        if (held)
            col = style.getColor(GUICol_FrameBgActive);
        else if (hovered)
            col = style.getColor(GUICol_FrameBgHovered);
        
        drawList->addRectFilled(bb.min, bb.max, col, style.FrameRounding);
        drawList->addRect(bb.min, bb.max, style.getColor(GUICol_Border), 
                         style.FrameRounding, style.FrameBorderSize);
        
        if (*v) {
            const float pad = 3.0f;
            drawList->addRectFilled(bb.min + Vec2(pad, pad), bb.max - Vec2(pad, pad), 
                                   style.getColor(GUICol_CheckMark), 0.0f);
        }
        
        // Draw label
        Vec2 labelPos = checkboxPos + Vec2(checkSize + style.ItemInnerSpacing.x, 
                                            (checkSize - 16.0f) * 0.5f);
        drawList->addText(labelPos, style.getColor(GUICol_Text), label);
    }
    
    ctx->dummy(Vec2(checkSize + style.ItemInnerSpacing.x + labelWidth, checkSize));
    ctx->newLine();
    
    return pressed;
}

bool checkboxFlags(const char* label, int* flags, int flagsValue) {
    bool v = (*flags & flagsValue) == flagsValue;
    bool pressed = checkbox(label, &v);
    if (pressed) {
        if (v)
            *flags |= flagsValue;
        else
            *flags &= ~flagsValue;
    }
    return pressed;
}

bool radioButton(const char* label, bool active) {
    GUIContext* ctx = getCurrentContext();
    if (!ctx) return false;
    
    const GUIStyle& style = ctx->getStyle();
    Vec2 pos = ctx->getCursorScreenPos();
    
    float radioSize = 16.0f;
    float labelWidth = std::strlen(label) * 8.0f;
    
    Vec2 radioPos = pos;
    Vec2 size(radioSize, radioSize);
    Rect bb(radioPos, radioPos + size);
    
    ID id = ctx->getID(label);
    bool hovered = false;
    bool held = false;
    bool pressed = ctx->buttonBehavior(bb, id, &hovered, &held);
    
    // Render radio button
    GUIDrawList* drawList = ctx->getWindowDrawList();
    if (drawList) {
        Color col = style.getColor(GUICol_FrameBg);
        if (held)
            col = style.getColor(GUICol_FrameBgActive);
        else if (hovered)
            col = style.getColor(GUICol_FrameBgHovered);
        
        Vec2 center = bb.getCenter();
        float radius = radioSize * 0.5f;
        
        drawList->addCircleFilled(center, radius, col);
        drawList->addCircle(center, radius, style.getColor(GUICol_Border), 0, 1.0f);
        
        if (active) {
            drawList->addCircleFilled(center, radius * 0.6f, style.getColor(GUICol_CheckMark));
        }
        
        // Draw label
        Vec2 labelPos = radioPos + Vec2(radioSize + style.ItemInnerSpacing.x, 
                                        (radioSize - 16.0f) * 0.5f);
        drawList->addText(labelPos, style.getColor(GUICol_Text), label);
    }
    
    ctx->dummy(Vec2(radioSize + style.ItemInnerSpacing.x + labelWidth, radioSize));
    ctx->newLine();
    
    return pressed;
}

bool radioButton(const char* label, int* v, int vButton) {
    bool pressed = radioButton(label, *v == vButton);
    if (pressed) {
        *v = vButton;
    }
    return pressed;
}

//-----------------------------------------------------------------------------
// Slider Widgets
//-----------------------------------------------------------------------------

bool sliderFloat(const char* label, float* v, float vMin, float vMax, const char* format) {
    GUIContext* ctx = getCurrentContext();
    if (!ctx || !v) return false;
    
    const GUIStyle& style = ctx->getStyle();
    Vec2 pos = ctx->getCursorScreenPos();
    
    float sliderWidth = 200.0f;
    float sliderHeight = 20.0f;
    float labelWidth = std::strlen(label) * 8.0f;
    
    // Draw label
    GUIDrawList* drawList = ctx->getWindowDrawList();
    if (drawList) {
        drawList->addText(pos, style.getColor(GUICol_Text), label);
    }
    
    Vec2 sliderPos = pos + Vec2(labelWidth + style.ItemInnerSpacing.x, 0);
    Vec2 sliderSize(sliderWidth, sliderHeight);
    Rect bb(sliderPos, sliderPos + sliderSize);
    
    ID id = ctx->getID(label);
    bool hovered = false;
    bool held = false;
    ctx->buttonBehavior(bb, id, &hovered, &held);
    
    bool valueChanged = false;
    
    if (held && ctx->isMouseHoveringRect(bb.min, bb.max)) {
        // Get mouse position relative to slider
        Vec2 mousePos = ctx->getCursorScreenPos();  // Would need actual mouse pos from context
        float t = (mousePos.x - bb.min.x) / bb.getWidth();
        t = std::max(0.0f, std::min(1.0f, t));
        
        float newValue = vMin + t * (vMax - vMin);
        if (newValue != *v) {
            *v = newValue;
            valueChanged = true;
        }
    }
    
    // Clamp value
    *v = std::max(vMin, std::min(*v, vMax));
    
    // Render slider
    if (drawList) {
        Color col = style.getColor(GUICol_FrameBg);
        if (held)
            col = style.getColor(GUICol_FrameBgActive);
        else if (hovered)
            col = style.getColor(GUICol_FrameBgHovered);
        
        drawList->addRectFilled(bb.min, bb.max, col, style.FrameRounding);
        drawList->addRect(bb.min, bb.max, style.getColor(GUICol_Border), 
                         style.FrameRounding, style.FrameBorderSize);
        
        // Draw grab
        float t = (*v - vMin) / (vMax - vMin);
        float grabWidth = 10.0f;
        float grabX = bb.min.x + t * (bb.getWidth() - grabWidth);
        
        Rect grabRect(Vec2(grabX, bb.min.y), Vec2(grabX + grabWidth, bb.max.y));
        Color grabCol = held ? style.getColor(GUICol_SliderGrabActive) : 
                              style.getColor(GUICol_SliderGrab);
        drawList->addRectFilled(grabRect.min, grabRect.max, grabCol, style.GrabRounding);
        
        // Draw value text
        char valueText[64];
        snprintf(valueText, sizeof(valueText), format, *v);
        Vec2 textPos = bb.min + Vec2((sliderWidth - std::strlen(valueText) * 8.0f) * 0.5f, 
                                     (sliderHeight - 16.0f) * 0.5f);
        drawList->addText(textPos, style.getColor(GUICol_Text), valueText);
    }
    
    ctx->dummy(Vec2(labelWidth + style.ItemInnerSpacing.x + sliderWidth, sliderHeight));
    ctx->newLine();
    
    return valueChanged;
}

bool sliderFloat2(const char* label, float v[2], float vMin, float vMax, const char* format) {
    bool changed = false;
    GUIContext* ctx = getCurrentContext();
    if (ctx) {
        ctx->pushID(label);
        changed |= sliderFloat("X", &v[0], vMin, vMax, format);
        changed |= sliderFloat("Y", &v[1], vMin, vMax, format);
        ctx->popID();
    }
    return changed;
}

bool sliderFloat3(const char* label, float v[3], float vMin, float vMax, const char* format) {
    bool changed = false;
    GUIContext* ctx = getCurrentContext();
    if (ctx) {
        ctx->pushID(label);
        changed |= sliderFloat("X", &v[0], vMin, vMax, format);
        changed |= sliderFloat("Y", &v[1], vMin, vMax, format);
        changed |= sliderFloat("Z", &v[2], vMin, vMax, format);
        ctx->popID();
    }
    return changed;
}

bool sliderFloat4(const char* label, float v[4], float vMin, float vMax, const char* format) {
    bool changed = false;
    GUIContext* ctx = getCurrentContext();
    if (ctx) {
        ctx->pushID(label);
        changed |= sliderFloat("X", &v[0], vMin, vMax, format);
        changed |= sliderFloat("Y", &v[1], vMin, vMax, format);
        changed |= sliderFloat("Z", &v[2], vMin, vMax, format);
        changed |= sliderFloat("W", &v[3], vMin, vMax, format);
        ctx->popID();
    }
    return changed;
}

bool sliderInt(const char* label, int* v, int vMin, int vMax, const char* format) {
    float fValue = static_cast<float>(*v);
    bool changed = sliderFloat(label, &fValue, static_cast<float>(vMin), 
                               static_cast<float>(vMax), format);
    if (changed) {
        *v = static_cast<int>(fValue);
    }
    return changed;
}

bool sliderInt2(const char* label, int v[2], int vMin, int vMax, const char* format) {
    bool changed = false;
    GUIContext* ctx = getCurrentContext();
    if (ctx) {
        ctx->pushID(label);
        changed |= sliderInt("X", &v[0], vMin, vMax, format);
        changed |= sliderInt("Y", &v[1], vMin, vMax, format);
        ctx->popID();
    }
    return changed;
}

bool sliderInt3(const char* label, int v[3], int vMin, int vMax, const char* format) {
    bool changed = false;
    GUIContext* ctx = getCurrentContext();
    if (ctx) {
        ctx->pushID(label);
        changed |= sliderInt("X", &v[0], vMin, vMax, format);
        changed |= sliderInt("Y", &v[1], vMin, vMax, format);
        changed |= sliderInt("Z", &v[2], vMin, vMax, format);
        ctx->popID();
    }
    return changed;
}

bool sliderInt4(const char* label, int v[4], int vMin, int vMax, const char* format) {
    bool changed = false;
    GUIContext* ctx = getCurrentContext();
    if (ctx) {
        ctx->pushID(label);
        changed |= sliderInt("X", &v[0], vMin, vMax, format);
        changed |= sliderInt("Y", &v[1], vMin, vMax, format);
        changed |= sliderInt("Z", &v[2], vMin, vMax, format);
        changed |= sliderInt("W", &v[3], vMin, vMax, format);
        ctx->popID();
    }
    return changed;
}

//-----------------------------------------------------------------------------
// Input Widgets (Simplified - full implementation would need text input handling)
//-----------------------------------------------------------------------------

bool inputText(const char* label, char* buf, size_t bufSize) {
    text("%s: [Input]", label);
    return false;
}

bool inputTextMultiline(const char* label, char* buf, size_t bufSize, const Vec2& size) {
    text("%s: [Multiline Input]", label);
    return false;
}

bool inputFloat(const char* label, float* v, float step, float stepFast, const char* format) {
    return sliderFloat(label, v, *v - 100.0f, *v + 100.0f, format);
}

bool inputInt(const char* label, int* v, int step, int stepFast) {
    return sliderInt(label, v, *v - 100, *v + 100, "%d");
}

//-----------------------------------------------------------------------------
// Color Widgets
//-----------------------------------------------------------------------------

bool colorEdit3(const char* label, float col[3]) {
    bool changed = false;
    GUIContext* ctx = getCurrentContext();
    if (ctx) {
        ctx->pushID(label);
        text("%s", label);
        changed |= sliderFloat("R", &col[0], 0.0f, 1.0f, "%.3f");
        changed |= sliderFloat("G", &col[1], 0.0f, 1.0f, "%.3f");
        changed |= sliderFloat("B", &col[2], 0.0f, 1.0f, "%.3f");
        ctx->popID();
    }
    return changed;
}

bool colorEdit4(const char* label, float col[4]) {
    bool changed = false;
    GUIContext* ctx = getCurrentContext();
    if (ctx) {
        ctx->pushID(label);
        text("%s", label);
        changed |= sliderFloat("R", &col[0], 0.0f, 1.0f, "%.3f");
        changed |= sliderFloat("G", &col[1], 0.0f, 1.0f, "%.3f");
        changed |= sliderFloat("B", &col[2], 0.0f, 1.0f, "%.3f");
        changed |= sliderFloat("A", &col[3], 0.0f, 1.0f, "%.3f");
        ctx->popID();
    }
    return changed;
}

bool colorPicker3(const char* label, float col[3]) {
    return colorEdit3(label, col);
}

bool colorPicker4(const char* label, float col[4]) {
    return colorEdit4(label, col);
}

//-----------------------------------------------------------------------------
// Tree Widgets
//-----------------------------------------------------------------------------

bool treeNode(const char* label) {
    return button(label);  // Simplified
}

bool treeNode(const char* strId, const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    formatText(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    return treeNode(buffer);
}

bool treeNodeEx(const char* label, int flags) {
    return treeNode(label);
}

void treePop() {
    GUIContext* ctx = getCurrentContext();
    if (ctx) {
        ctx->unindent();
    }
}

//-----------------------------------------------------------------------------
// Selectable Widgets
//-----------------------------------------------------------------------------

bool selectable(const char* label, bool selected, int flags, const Vec2& size) {
    return button(label, size);
}

bool selectable(const char* label, bool* pSelected, int flags, const Vec2& size) {
    bool pressed = selectable(label, *pSelected, flags, size);
    if (pressed) {
        *pSelected = !*pSelected;
    }
    return pressed;
}

//-----------------------------------------------------------------------------
// Combo Widgets
//-----------------------------------------------------------------------------

bool beginCombo(const char* label, const char* previewValue, int flags) {
    text("%s: %s", label, previewValue);
    return false;
}

void endCombo() {
}

bool combo(const char* label, int* currentItem, const char* const items[], int itemsCount, int popupMaxHeightInItems) {
    if (!items || itemsCount <= 0 || !currentItem)
        return false;
    
    const char* previewValue = (*currentItem >= 0 && *currentItem < itemsCount) ? 
                                items[*currentItem] : "";
    
    text("%s: %s", label, previewValue);
    return false;
}

//-----------------------------------------------------------------------------
// Menu Widgets
//-----------------------------------------------------------------------------

bool beginMenuBar() { return false; }
void endMenuBar() {}
bool beginMenu(const char* label, bool enabled) { return false; }
void endMenu() {}

bool menuItem(const char* label, const char* shortcut, bool selected, bool enabled) {
    return button(label);
}

bool menuItem(const char* label, const char* shortcut, bool* pSelected, bool enabled) {
    bool pressed = menuItem(label, shortcut, *pSelected, enabled);
    if (pressed) {
        *pSelected = !*pSelected;
    }
    return pressed;
}

//-----------------------------------------------------------------------------
// Tooltip Widgets
//-----------------------------------------------------------------------------

void beginTooltip() {}
void endTooltip() {}

void setTooltip(const char* fmt, ...) {
    // Simplified
}

//-----------------------------------------------------------------------------
// Popup Widgets
//-----------------------------------------------------------------------------

void openPopup(const char* strId) {}
bool beginPopup(const char* strId) { return false; }
void endPopup() {}
void closeCurrentPopup() {}

//-----------------------------------------------------------------------------
// Item Queries
//-----------------------------------------------------------------------------

bool isItemHovered() {
    GUIContext* ctx = getCurrentContext();
    if (ctx) return ctx->isItemHovered();
    return false;
}

bool isItemActive() {
    GUIContext* ctx = getCurrentContext();
    if (ctx) return ctx->isItemActive();
    return false;
}

bool isItemClicked(int mouseButton) {
    GUIContext* ctx = getCurrentContext();
    if (ctx) return ctx->isItemClicked(mouseButton);
    return false;
}

} // namespace CustomGUI
