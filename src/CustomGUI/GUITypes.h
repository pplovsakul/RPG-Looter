#pragma once

#include "../vendor/glm/glm.hpp"
#include <string>
#include <cstdint>

namespace CustomGUI {

// Forward declarations
class GUIContext;
class GUIDrawList;
class GUIStyle;

// Basic types
using ID = uint32_t;
using Vec2 = glm::vec2;
using Vec4 = glm::vec4;
using Color = glm::vec4;

// Widget state flags
enum GUIItemFlags {
    GUIItemFlags_None = 0,
    GUIItemFlags_Disabled = 1 << 0,
    GUIItemFlags_ReadOnly = 1 << 1,
};

// Mouse button indices
enum GUIMouseButton {
    GUIMouseButton_Left = 0,
    GUIMouseButton_Right = 1,
    GUIMouseButton_Middle = 2,
    GUIMouseButton_COUNT = 3
};

// Key indices
enum GUIKey {
    GUIKey_Tab,
    GUIKey_LeftArrow,
    GUIKey_RightArrow,
    GUIKey_UpArrow,
    GUIKey_DownArrow,
    GUIKey_PageUp,
    GUIKey_PageDown,
    GUIKey_Home,
    GUIKey_End,
    GUIKey_Insert,
    GUIKey_Delete,
    GUIKey_Backspace,
    GUIKey_Space,
    GUIKey_Enter,
    GUIKey_Escape,
    GUIKey_A,
    GUIKey_C,
    GUIKey_V,
    GUIKey_X,
    GUIKey_Y,
    GUIKey_Z,
    GUIKey_COUNT
};

// Window flags
enum GUIWindowFlags {
    GUIWindowFlags_None = 0,
    GUIWindowFlags_NoTitleBar = 1 << 0,
    GUIWindowFlags_NoResize = 1 << 1,
    GUIWindowFlags_NoMove = 1 << 2,
    GUIWindowFlags_NoScrollbar = 1 << 3,
    GUIWindowFlags_NoCollapse = 1 << 4,
    GUIWindowFlags_AlwaysAutoResize = 1 << 5,
    GUIWindowFlags_NoBackground = 1 << 6,
};

// Color indices for styling
enum GUICol {
    GUICol_Text,
    GUICol_TextDisabled,
    GUICol_WindowBg,
    GUICol_ChildBg,
    GUICol_PopupBg,
    GUICol_Border,
    GUICol_FrameBg,
    GUICol_FrameBgHovered,
    GUICol_FrameBgActive,
    GUICol_TitleBg,
    GUICol_TitleBgActive,
    GUICol_TitleBgCollapsed,
    GUICol_ScrollbarBg,
    GUICol_ScrollbarGrab,
    GUICol_ScrollbarGrabHovered,
    GUICol_ScrollbarGrabActive,
    GUICol_CheckMark,
    GUICol_SliderGrab,
    GUICol_SliderGrabActive,
    GUICol_Button,
    GUICol_ButtonHovered,
    GUICol_ButtonActive,
    GUICol_Header,
    GUICol_HeaderHovered,
    GUICol_HeaderActive,
    GUICol_Separator,
    GUICol_SeparatorHovered,
    GUICol_SeparatorActive,
    GUICol_ResizeGrip,
    GUICol_ResizeGripHovered,
    GUICol_ResizeGripActive,
    GUICol_Tab,
    GUICol_TabHovered,
    GUICol_TabActive,
    GUICol_TabUnfocused,
    GUICol_TabUnfocusedActive,
    GUICol_PlotLines,
    GUICol_PlotLinesHovered,
    GUICol_PlotHistogram,
    GUICol_PlotHistogramHovered,
    GUICol_TextSelectedBg,
    GUICol_DragDropTarget,
    GUICol_NavHighlight,
    GUICol_NavWindowingHighlight,
    GUICol_NavWindowingDimBg,
    GUICol_ModalWindowDimBg,
    GUICol_COUNT
};

// Style variable indices
enum GUIStyleVar {
    GUIStyleVar_Alpha,
    GUIStyleVar_WindowPadding,
    GUIStyleVar_WindowRounding,
    GUIStyleVar_WindowBorderSize,
    GUIStyleVar_WindowMinSize,
    GUIStyleVar_ChildRounding,
    GUIStyleVar_ChildBorderSize,
    GUIStyleVar_PopupRounding,
    GUIStyleVar_PopupBorderSize,
    GUIStyleVar_FramePadding,
    GUIStyleVar_FrameRounding,
    GUIStyleVar_FrameBorderSize,
    GUIStyleVar_ItemSpacing,
    GUIStyleVar_ItemInnerSpacing,
    GUIStyleVar_IndentSpacing,
    GUIStyleVar_ScrollbarSize,
    GUIStyleVar_ScrollbarRounding,
    GUIStyleVar_GrabMinSize,
    GUIStyleVar_GrabRounding,
    GUIStyleVar_TabRounding,
    GUIStyleVar_ButtonTextAlign,
    GUIStyleVar_SelectableTextAlign,
    GUIStyleVar_COUNT
};

// Rectangle structure
struct Rect {
    Vec2 min;
    Vec2 max;

    Rect() : min(0, 0), max(0, 0) {}
    Rect(const Vec2& min, const Vec2& max) : min(min), max(max) {}
    Rect(float x1, float y1, float x2, float y2) : min(x1, y1), max(x2, y2) {}

    Vec2 getCenter() const { return (min + max) * 0.5f; }
    Vec2 getSize() const { return max - min; }
    float getWidth() const { return max.x - min.x; }
    float getHeight() const { return max.y - min.y; }

    bool contains(const Vec2& p) const {
        return p.x >= min.x && p.y >= min.y && p.x < max.x && p.y < max.y;
    }

    void expand(const Vec2& amount) {
        min -= amount;
        max += amount;
    }

    void clip(const Rect& r) {
        min.x = glm::max(min.x, r.min.x);
        min.y = glm::max(min.y, r.min.y);
        max.x = glm::min(max.x, r.max.x);
        max.y = glm::min(max.y, r.max.y);
    }
};

} // namespace CustomGUI
