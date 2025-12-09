#include "GUIStyle.h"

namespace CustomGUI {

GUIStyle::GUIStyle() {
    // Initialize with dark theme by default
    setDarkTheme();
}

void GUIStyle::setColor(GUICol idx, const Color& color) {
    if (idx >= 0 && idx < GUICol_COUNT) {
        Colors[idx] = color;
    }
}

Color GUIStyle::getColor(GUICol idx) const {
    if (idx >= 0 && idx < GUICol_COUNT) {
        return Colors[idx];
    }
    return Color(1, 1, 1, 1);
}

void GUIStyle::setDarkTheme() {
    Colors[GUICol_Text]                  = Color(1.00f, 1.00f, 1.00f, 1.00f);
    Colors[GUICol_TextDisabled]          = Color(0.50f, 0.50f, 0.50f, 1.00f);
    Colors[GUICol_WindowBg]              = Color(0.06f, 0.06f, 0.06f, 0.94f);
    Colors[GUICol_ChildBg]               = Color(0.00f, 0.00f, 0.00f, 0.00f);
    Colors[GUICol_PopupBg]               = Color(0.08f, 0.08f, 0.08f, 0.94f);
    Colors[GUICol_Border]                = Color(0.43f, 0.43f, 0.50f, 0.50f);
    Colors[GUICol_FrameBg]               = Color(0.16f, 0.29f, 0.48f, 0.54f);
    Colors[GUICol_FrameBgHovered]        = Color(0.26f, 0.59f, 0.98f, 0.40f);
    Colors[GUICol_FrameBgActive]         = Color(0.26f, 0.59f, 0.98f, 0.67f);
    Colors[GUICol_TitleBg]               = Color(0.04f, 0.04f, 0.04f, 1.00f);
    Colors[GUICol_TitleBgActive]         = Color(0.16f, 0.29f, 0.48f, 1.00f);
    Colors[GUICol_TitleBgCollapsed]      = Color(0.00f, 0.00f, 0.00f, 0.51f);
    Colors[GUICol_ScrollbarBg]           = Color(0.02f, 0.02f, 0.02f, 0.53f);
    Colors[GUICol_ScrollbarGrab]         = Color(0.31f, 0.31f, 0.31f, 1.00f);
    Colors[GUICol_ScrollbarGrabHovered]  = Color(0.41f, 0.41f, 0.41f, 1.00f);
    Colors[GUICol_ScrollbarGrabActive]   = Color(0.51f, 0.51f, 0.51f, 1.00f);
    Colors[GUICol_CheckMark]             = Color(0.26f, 0.59f, 0.98f, 1.00f);
    Colors[GUICol_SliderGrab]            = Color(0.24f, 0.52f, 0.88f, 1.00f);
    Colors[GUICol_SliderGrabActive]      = Color(0.26f, 0.59f, 0.98f, 1.00f);
    Colors[GUICol_Button]                = Color(0.26f, 0.59f, 0.98f, 0.40f);
    Colors[GUICol_ButtonHovered]         = Color(0.26f, 0.59f, 0.98f, 1.00f);
    Colors[GUICol_ButtonActive]          = Color(0.06f, 0.53f, 0.98f, 1.00f);
    Colors[GUICol_Header]                = Color(0.26f, 0.59f, 0.98f, 0.31f);
    Colors[GUICol_HeaderHovered]         = Color(0.26f, 0.59f, 0.98f, 0.80f);
    Colors[GUICol_HeaderActive]          = Color(0.26f, 0.59f, 0.98f, 1.00f);
    Colors[GUICol_Separator]             = Colors[GUICol_Border];
    Colors[GUICol_SeparatorHovered]      = Color(0.10f, 0.40f, 0.75f, 0.78f);
    Colors[GUICol_SeparatorActive]       = Color(0.10f, 0.40f, 0.75f, 1.00f);
    Colors[GUICol_ResizeGrip]            = Color(0.26f, 0.59f, 0.98f, 0.25f);
    Colors[GUICol_ResizeGripHovered]     = Color(0.26f, 0.59f, 0.98f, 0.67f);
    Colors[GUICol_ResizeGripActive]      = Color(0.26f, 0.59f, 0.98f, 0.95f);
    Colors[GUICol_Tab]                   = Color(0.18f, 0.35f, 0.58f, 0.86f);
    Colors[GUICol_TabHovered]            = Color(0.26f, 0.59f, 0.98f, 0.80f);
    Colors[GUICol_TabActive]             = Color(0.20f, 0.41f, 0.68f, 1.00f);
    Colors[GUICol_TabUnfocused]          = Color(0.07f, 0.10f, 0.15f, 0.97f);
    Colors[GUICol_TabUnfocusedActive]    = Color(0.14f, 0.26f, 0.42f, 1.00f);
    Colors[GUICol_PlotLines]             = Color(0.61f, 0.61f, 0.61f, 1.00f);
    Colors[GUICol_PlotLinesHovered]      = Color(1.00f, 0.43f, 0.35f, 1.00f);
    Colors[GUICol_PlotHistogram]         = Color(0.90f, 0.70f, 0.00f, 1.00f);
    Colors[GUICol_PlotHistogramHovered]  = Color(1.00f, 0.60f, 0.00f, 1.00f);
    Colors[GUICol_TextSelectedBg]        = Color(0.26f, 0.59f, 0.98f, 0.35f);
    Colors[GUICol_DragDropTarget]        = Color(1.00f, 1.00f, 0.00f, 0.90f);
    Colors[GUICol_NavHighlight]          = Color(0.26f, 0.59f, 0.98f, 1.00f);
    Colors[GUICol_NavWindowingHighlight] = Color(1.00f, 1.00f, 1.00f, 0.70f);
    Colors[GUICol_NavWindowingDimBg]     = Color(0.80f, 0.80f, 0.80f, 0.20f);
    Colors[GUICol_ModalWindowDimBg]      = Color(0.80f, 0.80f, 0.80f, 0.35f);
}

void GUIStyle::setLightTheme() {
    Colors[GUICol_Text]                  = Color(0.00f, 0.00f, 0.00f, 1.00f);
    Colors[GUICol_TextDisabled]          = Color(0.60f, 0.60f, 0.60f, 1.00f);
    Colors[GUICol_WindowBg]              = Color(0.94f, 0.94f, 0.94f, 1.00f);
    Colors[GUICol_ChildBg]               = Color(0.00f, 0.00f, 0.00f, 0.00f);
    Colors[GUICol_PopupBg]               = Color(1.00f, 1.00f, 1.00f, 0.98f);
    Colors[GUICol_Border]                = Color(0.00f, 0.00f, 0.00f, 0.30f);
    Colors[GUICol_FrameBg]               = Color(1.00f, 1.00f, 1.00f, 1.00f);
    Colors[GUICol_FrameBgHovered]        = Color(0.26f, 0.59f, 0.98f, 0.40f);
    Colors[GUICol_FrameBgActive]         = Color(0.26f, 0.59f, 0.98f, 0.67f);
    Colors[GUICol_TitleBg]               = Color(0.96f, 0.96f, 0.96f, 1.00f);
    Colors[GUICol_TitleBgActive]         = Color(0.82f, 0.82f, 0.82f, 1.00f);
    Colors[GUICol_TitleBgCollapsed]      = Color(1.00f, 1.00f, 1.00f, 0.51f);
    Colors[GUICol_ScrollbarBg]           = Color(0.98f, 0.98f, 0.98f, 0.53f);
    Colors[GUICol_ScrollbarGrab]         = Color(0.69f, 0.69f, 0.69f, 0.80f);
    Colors[GUICol_ScrollbarGrabHovered]  = Color(0.49f, 0.49f, 0.49f, 0.80f);
    Colors[GUICol_ScrollbarGrabActive]   = Color(0.49f, 0.49f, 0.49f, 1.00f);
    Colors[GUICol_CheckMark]             = Color(0.26f, 0.59f, 0.98f, 1.00f);
    Colors[GUICol_SliderGrab]            = Color(0.26f, 0.59f, 0.98f, 0.78f);
    Colors[GUICol_SliderGrabActive]      = Color(0.46f, 0.54f, 0.80f, 0.60f);
    Colors[GUICol_Button]                = Color(0.26f, 0.59f, 0.98f, 0.40f);
    Colors[GUICol_ButtonHovered]         = Color(0.26f, 0.59f, 0.98f, 1.00f);
    Colors[GUICol_ButtonActive]          = Color(0.06f, 0.53f, 0.98f, 1.00f);
    Colors[GUICol_Header]                = Color(0.26f, 0.59f, 0.98f, 0.31f);
    Colors[GUICol_HeaderHovered]         = Color(0.26f, 0.59f, 0.98f, 0.80f);
    Colors[GUICol_HeaderActive]          = Color(0.26f, 0.59f, 0.98f, 1.00f);
    Colors[GUICol_Separator]             = Color(0.39f, 0.39f, 0.39f, 0.62f);
    Colors[GUICol_SeparatorHovered]      = Color(0.14f, 0.44f, 0.80f, 0.78f);
    Colors[GUICol_SeparatorActive]       = Color(0.14f, 0.44f, 0.80f, 1.00f);
    Colors[GUICol_ResizeGrip]            = Color(0.35f, 0.35f, 0.35f, 0.17f);
    Colors[GUICol_ResizeGripHovered]     = Color(0.26f, 0.59f, 0.98f, 0.67f);
    Colors[GUICol_ResizeGripActive]      = Color(0.26f, 0.59f, 0.98f, 0.95f);
    Colors[GUICol_Tab]                   = Color(0.76f, 0.80f, 0.84f, 0.93f);
    Colors[GUICol_TabHovered]            = Color(0.26f, 0.59f, 0.98f, 0.80f);
    Colors[GUICol_TabActive]             = Color(0.60f, 0.73f, 0.88f, 1.00f);
    Colors[GUICol_TabUnfocused]          = Color(0.92f, 0.93f, 0.94f, 0.98f);
    Colors[GUICol_TabUnfocusedActive]    = Color(0.74f, 0.82f, 0.91f, 1.00f);
    Colors[GUICol_PlotLines]             = Color(0.39f, 0.39f, 0.39f, 1.00f);
    Colors[GUICol_PlotLinesHovered]      = Color(1.00f, 0.43f, 0.35f, 1.00f);
    Colors[GUICol_PlotHistogram]         = Color(0.90f, 0.70f, 0.00f, 1.00f);
    Colors[GUICol_PlotHistogramHovered]  = Color(1.00f, 0.45f, 0.00f, 1.00f);
    Colors[GUICol_TextSelectedBg]        = Color(0.26f, 0.59f, 0.98f, 0.35f);
    Colors[GUICol_DragDropTarget]        = Color(0.26f, 0.59f, 0.98f, 0.95f);
    Colors[GUICol_NavHighlight]          = Color(0.26f, 0.59f, 0.98f, 0.80f);
    Colors[GUICol_NavWindowingHighlight] = Color(0.70f, 0.70f, 0.70f, 0.70f);
    Colors[GUICol_NavWindowingDimBg]     = Color(0.20f, 0.20f, 0.20f, 0.20f);
    Colors[GUICol_ModalWindowDimBg]      = Color(0.20f, 0.20f, 0.20f, 0.35f);
}

void GUIStyle::setClassicTheme() {
    Colors[GUICol_Text]                  = Color(0.90f, 0.90f, 0.90f, 1.00f);
    Colors[GUICol_TextDisabled]          = Color(0.60f, 0.60f, 0.60f, 1.00f);
    Colors[GUICol_WindowBg]              = Color(0.00f, 0.00f, 0.00f, 0.70f);
    Colors[GUICol_ChildBg]               = Color(0.00f, 0.00f, 0.00f, 0.00f);
    Colors[GUICol_PopupBg]               = Color(0.11f, 0.11f, 0.14f, 0.92f);
    Colors[GUICol_Border]                = Color(0.50f, 0.50f, 0.50f, 0.50f);
    Colors[GUICol_FrameBg]               = Color(0.43f, 0.43f, 0.43f, 0.39f);
    Colors[GUICol_FrameBgHovered]        = Color(0.47f, 0.47f, 0.69f, 0.40f);
    Colors[GUICol_FrameBgActive]         = Color(0.42f, 0.41f, 0.64f, 0.69f);
    Colors[GUICol_TitleBg]               = Color(0.27f, 0.27f, 0.54f, 0.83f);
    Colors[GUICol_TitleBgActive]         = Color(0.32f, 0.32f, 0.63f, 0.87f);
    Colors[GUICol_TitleBgCollapsed]      = Color(0.40f, 0.40f, 0.80f, 0.20f);
    Colors[GUICol_ScrollbarBg]           = Color(0.20f, 0.25f, 0.30f, 0.60f);
    Colors[GUICol_ScrollbarGrab]         = Color(0.40f, 0.40f, 0.80f, 0.30f);
    Colors[GUICol_ScrollbarGrabHovered]  = Color(0.40f, 0.40f, 0.80f, 0.40f);
    Colors[GUICol_ScrollbarGrabActive]   = Color(0.41f, 0.39f, 0.80f, 0.60f);
    Colors[GUICol_CheckMark]             = Color(0.90f, 0.90f, 0.90f, 0.50f);
    Colors[GUICol_SliderGrab]            = Color(1.00f, 1.00f, 1.00f, 0.30f);
    Colors[GUICol_SliderGrabActive]      = Color(0.41f, 0.39f, 0.80f, 0.60f);
    Colors[GUICol_Button]                = Color(0.35f, 0.40f, 0.61f, 0.62f);
    Colors[GUICol_ButtonHovered]         = Color(0.40f, 0.48f, 0.71f, 0.79f);
    Colors[GUICol_ButtonActive]          = Color(0.46f, 0.54f, 0.80f, 1.00f);
    Colors[GUICol_Header]                = Color(0.40f, 0.40f, 0.90f, 0.45f);
    Colors[GUICol_HeaderHovered]         = Color(0.45f, 0.45f, 0.90f, 0.80f);
    Colors[GUICol_HeaderActive]          = Color(0.53f, 0.53f, 0.87f, 0.80f);
    Colors[GUICol_Separator]             = Color(0.50f, 0.50f, 0.50f, 0.60f);
    Colors[GUICol_SeparatorHovered]      = Color(0.60f, 0.60f, 0.70f, 1.00f);
    Colors[GUICol_SeparatorActive]       = Color(0.70f, 0.70f, 0.90f, 1.00f);
    Colors[GUICol_ResizeGrip]            = Color(1.00f, 1.00f, 1.00f, 0.16f);
    Colors[GUICol_ResizeGripHovered]     = Color(0.78f, 0.82f, 1.00f, 0.60f);
    Colors[GUICol_ResizeGripActive]      = Color(0.78f, 0.82f, 1.00f, 0.90f);
    Colors[GUICol_Tab]                   = Color(0.34f, 0.34f, 0.68f, 0.79f);
    Colors[GUICol_TabHovered]            = Color(0.45f, 0.45f, 0.90f, 0.80f);
    Colors[GUICol_TabActive]             = Color(0.40f, 0.40f, 0.73f, 0.84f);
    Colors[GUICol_TabUnfocused]          = Color(0.28f, 0.28f, 0.57f, 0.82f);
    Colors[GUICol_TabUnfocusedActive]    = Color(0.35f, 0.35f, 0.65f, 0.84f);
    Colors[GUICol_PlotLines]             = Color(1.00f, 1.00f, 1.00f, 1.00f);
    Colors[GUICol_PlotLinesHovered]      = Color(0.90f, 0.70f, 0.00f, 1.00f);
    Colors[GUICol_PlotHistogram]         = Color(0.90f, 0.70f, 0.00f, 1.00f);
    Colors[GUICol_PlotHistogramHovered]  = Color(1.00f, 0.60f, 0.00f, 1.00f);
    Colors[GUICol_TextSelectedBg]        = Color(0.00f, 0.00f, 1.00f, 0.35f);
    Colors[GUICol_DragDropTarget]        = Color(1.00f, 1.00f, 0.00f, 0.90f);
    Colors[GUICol_NavHighlight]          = Color(0.45f, 0.45f, 0.90f, 0.80f);
    Colors[GUICol_NavWindowingHighlight] = Color(1.00f, 1.00f, 1.00f, 0.70f);
    Colors[GUICol_NavWindowingDimBg]     = Color(0.80f, 0.80f, 0.80f, 0.20f);
    Colors[GUICol_ModalWindowDimBg]      = Color(0.20f, 0.20f, 0.20f, 0.35f);
}

} // namespace CustomGUI
