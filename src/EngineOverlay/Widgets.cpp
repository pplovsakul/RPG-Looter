#include "Widgets.h"
#include <cstring>
#include <algorithm>
#include <cstdio>

namespace EngineUI {

// ============================================================================
// Basic Widgets Implementation
// ============================================================================

void Text(const char* text) {
    UIContext* ctx = GetContext();
    if (!ctx || !text) return;
    
    Style& style = ctx->getStyle();
    glm::vec2 textSize = ctx->measureText(text);
    
    Rect rect = ctx->allocRect(textSize.x, textSize.y);
    
    // Draw actual text
    ctx->drawText(glm::vec2(rect.x, rect.y), style.text, text);
}

void TextColored(const Color& color, const char* text) {
    UIContext* ctx = GetContext();
    if (!ctx || !text) return;
    
    glm::vec2 textSize = ctx->measureText(text);
    Rect rect = ctx->allocRect(textSize.x, textSize.y);
    
    ctx->drawText(glm::vec2(rect.x, rect.y), color, text);
}

bool Button(const char* label, float width, float height) {
    UIContext* ctx = GetContext();
    if (!ctx) return false;
    
    Style& style = ctx->getStyle();
    
    glm::vec2 textSize = ctx->measureText(label);
    if (width == 0.0f) width = textSize.x + style.windowPadding * 2;
    if (height == 0.0f) height = textSize.y + style.windowPadding;
    
    Rect rect = ctx->allocRect(width, height);
    WidgetID id = ctx->getID(label);
    ctx->setLastItemID(id);
    
    bool hovered = ctx->isHovered(rect);
    bool held = false;
    
    if (hovered && ctx->isMouseDown(0)) {
        ctx->setActive(id);
        held = true;
    }
    
    bool pressed = false;
    if (hovered && ctx->isMouseReleased(0) && ctx->isActive(id)) {
        pressed = true;
        ctx->clearActive();
    }
    
    // Draw button
    DrawList& draw = ctx->getDrawList();
    Color btnColor = held ? style.buttonActive : (hovered ? style.buttonHovered : style.button);
    draw.addRectFilled(rect, btnColor, style.frameRounding);
    draw.addRect(rect, style.border, style.frameRounding);
    
    // Draw label centered
    float textX = rect.x + (rect.w - textSize.x) * 0.5f;
    float textY = rect.y + (rect.h - textSize.y) * 0.5f;
    ctx->drawText(glm::vec2(textX, textY), style.text, label);
    
    return pressed;
}

bool Checkbox(const char* label, bool* v) {
    UIContext* ctx = GetContext();
    if (!ctx || !v) return false;
    
    Style& style = ctx->getStyle();
    float boxSize = style.lineHeight;
    float totalWidth = boxSize + style.itemInnerSpacing + std::strlen(label) * 7.0f;
    
    Rect rect = ctx->allocRect(totalWidth, boxSize);
    WidgetID id = ctx->getID(label);
    ctx->setLastItemID(id);
    
    Rect boxRect(rect.x, rect.y, boxSize, boxSize);
    bool hovered = ctx->isHovered(boxRect);
    
    bool pressed = false;
    if (hovered && ctx->isMouseClicked(0)) {
        *v = !*v;
        pressed = true;
    }
    
    // Draw checkbox
    DrawList& draw = ctx->getDrawList();
    Color bgColor = hovered ? style.frameBgHovered : style.frameBg;
    draw.addRectFilled(boxRect, bgColor, style.frameRounding);
    draw.addRect(boxRect, style.border, style.frameRounding);
    
    // Draw checkmark if checked
    if (*v) {
        float pad = boxSize * 0.25f;
        Rect checkRect(boxRect.x + pad, boxRect.y + pad, boxSize - 2*pad, boxSize - 2*pad);
        draw.addRectFilled(checkRect, style.checkMark);
    }
    
    // Draw label
    float textX = rect.x + boxSize + style.itemInnerSpacing;
    float textY = rect.y;
    float textWidth = std::strlen(label) * 7.0f;
    draw.addRectFilled(Rect(textX, textY, textWidth, style.lineHeight * 0.8f), style.text);
    
    return pressed;
}

bool RadioButton(const char* label, bool active) {
    UIContext* ctx = GetContext();
    if (!ctx) return false;
    
    Style& style = ctx->getStyle();
    float boxSize = style.lineHeight;
    float totalWidth = boxSize + style.itemInnerSpacing + std::strlen(label) * 7.0f;
    
    Rect rect = ctx->allocRect(totalWidth, boxSize);
    WidgetID id = ctx->getID(label);
    ctx->setLastItemID(id);
    
    Rect boxRect(rect.x, rect.y, boxSize, boxSize);
    bool hovered = ctx->isHovered(boxRect);
    
    bool pressed = false;
    if (hovered && ctx->isMouseClicked(0)) {
        pressed = true;
    }
    
    // Draw radio button (circle)
    DrawList& draw = ctx->getDrawList();
    Color bgColor = hovered ? style.frameBgHovered : style.frameBg;
    float radius = boxSize * 0.5f;
    glm::vec2 center(boxRect.x + radius, boxRect.y + radius);
    draw.addCircleFilled(center, radius, bgColor, 12);
    draw.addCircle(center, radius, style.border, 12);
    
    // Draw dot if active
    if (active) {
        draw.addCircleFilled(center, radius * 0.6f, style.checkMark, 8);
    }
    
    // Draw label
    float textX = rect.x + boxSize + style.itemInnerSpacing;
    float textY = rect.y;
    float textWidth = std::strlen(label) * 7.0f;
    draw.addRectFilled(Rect(textX, textY, textWidth, style.lineHeight * 0.8f), style.text);
    
    return pressed;
}

bool SliderFloat(const char* label, float* v, float v_min, float v_max) {
    UIContext* ctx = GetContext();
    if (!ctx || !v) return false;
    
    Style& style = ctx->getStyle();
    float sliderWidth = 150.0f;
    float height = style.lineHeight + style.windowPadding;
    
    Rect rect = ctx->allocRect(sliderWidth, height);
    WidgetID id = ctx->getID(label);
    ctx->setLastItemID(id);
    
    bool hovered = ctx->isHovered(rect);
    bool changed = false;
    
    // Handle input
    if (hovered && ctx->isMouseDown(0)) {
        ctx->setActive(id);
    }
    
    if (ctx->isActive(id)) {
        if (ctx->isMouseDown(0)) {
            float mouseX = ctx->getMousePos().x;
            float t = (mouseX - rect.x) / rect.w;
            t = std::max(0.0f, std::min(1.0f, t));
            float newValue = v_min + t * (v_max - v_min);
            if (*v != newValue) {
                *v = newValue;
                changed = true;
            }
        } else {
            ctx->clearActive();
        }
    }
    
    // Draw slider
    DrawList& draw = ctx->getDrawList();
    Color bgColor = hovered ? style.frameBgHovered : style.frameBg;
    draw.addRectFilled(rect, bgColor, style.frameRounding);
    draw.addRect(rect, style.border, style.frameRounding);
    
    // Draw filled portion
    float t = (*v - v_min) / (v_max - v_min);
    t = std::max(0.0f, std::min(1.0f, t));
    Rect filledRect(rect.x, rect.y, rect.w * t, rect.h);
    draw.addRectFilled(filledRect, style.buttonActive, style.frameRounding);
    
    // Draw grab
    float grabX = rect.x + rect.w * t;
    float grabSize = style.grabMinSize;
    Rect grabRect(grabX - grabSize * 0.5f, rect.y, grabSize, rect.h);
    draw.addRectFilled(grabRect, style.button);
    
    return changed;
}

bool SliderInt(const char* label, int* v, int v_min, int v_max) {
    float fval = (float)*v;
    bool changed = SliderFloat(label, &fval, (float)v_min, (float)v_max);
    if (changed) {
        *v = (int)fval;
    }
    return changed;
}

bool DragFloat(const char* label, float* v, float speed, float v_min, float v_max) {
    // Simplified drag - similar to slider for now
    return SliderFloat(label, v, v_min, v_max);
}

bool DragInt(const char* label, int* v, float speed, int v_min, int v_max) {
    return SliderInt(label, v, v_min, v_max);
}

bool InputText(const char* label, char* buf, size_t buf_size) {
    UIContext* ctx = GetContext();
    if (!ctx || !buf) return false;
    
    Style& style = ctx->getStyle();
    float width = 200.0f;
    float height = style.lineHeight + style.windowPadding;
    
    Rect rect = ctx->allocRect(width, height);
    WidgetID id = ctx->getID(label);
    ctx->setLastItemID(id);
    
    bool hovered = ctx->isHovered(rect);
    bool changed = false;
    
    if (hovered && ctx->isMouseClicked(0)) {
        ctx->setActive(id);
    }
    
    // Handle text input if active
    bool isActive = ctx->isActive(id);
    if (isActive) {
        const InputState& input = ctx->getInput();
        for (int i = 0; i < input.inputCharCount; ++i) {
            char c = input.inputCharacters[i];
            size_t len = std::strlen(buf);
            if (c >= 32 && c < 127 && len < buf_size - 1) {
                buf[len] = c;
                buf[len + 1] = '\0';
                changed = true;
            }
        }
        
        // Handle backspace (key code 259 in GLFW)
        if (input.keysPressed[259]) {
            size_t len = std::strlen(buf);
            if (len > 0) {
                buf[len - 1] = '\0';
                changed = true;
            }
        }
    }
    
    // Draw input box
    DrawList& draw = ctx->getDrawList();
    Color bgColor = isActive ? style.frameBgActive : (hovered ? style.frameBgHovered : style.frameBg);
    draw.addRectFilled(rect, bgColor, style.frameRounding);
    draw.addRect(rect, style.border, style.frameRounding);
    
    // Draw text
    if (buf[0] != '\0') {
        float textWidth = std::strlen(buf) * 7.0f;
        draw.addRectFilled(Rect(rect.x + 4, rect.y + 4, textWidth, style.lineHeight * 0.8f), style.text);
    }
    
    return changed;
}

bool ColorEdit3(const char* label, float col[3]) {
    // Simplified color edit - just show the color for now
    UIContext* ctx = GetContext();
    if (!ctx || !col) return false;
    
    Style& style = ctx->getStyle();
    float boxSize = style.lineHeight * 2;
    
    Rect rect = ctx->allocRect(boxSize, boxSize);
    WidgetID id = ctx->getID(label);
    ctx->setLastItemID(id);
    
    DrawList& draw = ctx->getDrawList();
    draw.addRectFilled(rect, Color(col[0], col[1], col[2], 1.0f));
    draw.addRect(rect, style.border);
    
    // TODO: Implement actual color picker
    return false;
}

bool ColorEdit4(const char* label, float col[4]) {
    UIContext* ctx = GetContext();
    if (!ctx || !col) return false;
    
    Style& style = ctx->getStyle();
    float boxSize = style.lineHeight * 2;
    
    Rect rect = ctx->allocRect(boxSize, boxSize);
    WidgetID id = ctx->getID(label);
    ctx->setLastItemID(id);
    
    DrawList& draw = ctx->getDrawList();
    draw.addRectFilled(rect, Color(col[0], col[1], col[2], col[3]));
    draw.addRect(rect, style.border);
    
    return false;
}

void Separator() {
    UIContext* ctx = GetContext();
    if (!ctx) return;
    
    Style& style = ctx->getStyle();
    Rect rect = ctx->allocRect(0, 1.0f);
    
    DrawList& draw = ctx->getDrawList();
    draw.addLine(glm::vec2(rect.x, rect.y), glm::vec2(rect.right(), rect.y), style.border);
}

void SameLine(float offset) {
    UIContext* ctx = GetContext();
    if (ctx) {
        ctx->sameLine(offset);
    }
}

void Spacing() {
    UIContext* ctx = GetContext();
    if (!ctx) return;
    
    Style& style = ctx->getStyle();
    ctx->allocRect(0, style.itemSpacing);
}

void Indent(float amount) {
    UIContext* ctx = GetContext();
    if (ctx) {
        ctx->indent(amount);
    }
}

void Unindent(float amount) {
    UIContext* ctx = GetContext();
    if (ctx) {
        ctx->unindent(amount);
    }
}

bool CollapsingHeader(const char* label) {
    // TODO: Implement collapsing header with state tracking
    UIContext* ctx = GetContext();
    if (!ctx) return false;
    
    // For now, just draw a button-like header that always returns true
    return Button(label);
}

bool TreeNode(const char* label) {
    // TODO: Implement tree node with state tracking
    return CollapsingHeader(label);
}

void TreePop() {
    Unindent();
}

bool BeginChild(const char* str_id, float width, float height) {
    // TODO: Implement child regions with scrolling
    UIContext* ctx = GetContext();
    if (!ctx) return false;
    
    // For now, just allocate space and indent
    ctx->allocRect(width, height);
    Indent();
    return true;
}

void EndChild() {
    Unindent();
}

void ProgressBar(float fraction, float width, const char* overlay) {
    UIContext* ctx = GetContext();
    if (!ctx) return;
    
    Style& style = ctx->getStyle();
    if (width < 0) width = 200.0f;
    float height = style.lineHeight;
    
    Rect rect = ctx->allocRect(width, height);
    
    DrawList& draw = ctx->getDrawList();
    draw.addRectFilled(rect, style.frameBg);
    
    float filledWidth = rect.w * std::max(0.0f, std::min(1.0f, fraction));
    Rect filledRect(rect.x, rect.y, filledWidth, rect.h);
    draw.addRectFilled(filledRect, style.buttonActive);
    
    draw.addRect(rect, style.border);
}

bool BeginCombo(const char* label, const char* preview) {
    // TODO: Implement combo box popup
    return false;
}

void EndCombo() {
    // TODO: Implement combo box popup end
}

bool Combo(const char* label, int* current, const char* const* items, int count) {
    // TODO: Implement full combo box
    return false;
}

void HelpMarker(const char* desc) {
    Text("(?)");
    // TODO: Show tooltip on hover
}

} // namespace EngineUI
