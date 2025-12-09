#pragma once

#include "GUITypes.h"
#include <array>

namespace CustomGUI {

// Style configuration for the GUI system
class GUIStyle {
public:
    // Sizes
    float Alpha = 1.0f;                         // Global alpha
    Vec2 WindowPadding = Vec2(8, 8);            // Padding within a window
    float WindowRounding = 0.0f;                // Radius of window corners rounding
    float WindowBorderSize = 1.0f;              // Thickness of border around windows
    Vec2 WindowMinSize = Vec2(32, 32);          // Minimum window size
    Vec2 WindowTitleAlign = Vec2(0.0f, 0.5f);   // Alignment for title bar text
    
    float ChildRounding = 0.0f;                 // Radius of child window corners
    float ChildBorderSize = 1.0f;               // Thickness of border around child windows
    
    float PopupRounding = 0.0f;                 // Radius of popup window corners
    float PopupBorderSize = 1.0f;               // Thickness of border around popups
    
    Vec2 FramePadding = Vec2(4, 3);             // Padding within a framed rectangle (used by most widgets)
    float FrameRounding = 0.0f;                 // Radius of frame corners rounding
    float FrameBorderSize = 0.0f;               // Thickness of border around frames
    
    Vec2 ItemSpacing = Vec2(8, 4);              // Horizontal and vertical spacing between widgets/lines
    Vec2 ItemInnerSpacing = Vec2(4, 4);         // Horizontal and vertical spacing between within elements
    Vec2 TouchExtraPadding = Vec2(0, 0);        // Expand reactive bounding box for touch-based system
    
    float IndentSpacing = 21.0f;                // Horizontal indentation when e.g. entering a tree node
    float ColumnsMinSpacing = 6.0f;             // Minimum horizontal spacing between two columns
    
    float ScrollbarSize = 14.0f;                // Width of the vertical scrollbar, Height of the horizontal scrollbar
    float ScrollbarRounding = 9.0f;             // Radius of grab corners for scrollbar
    float GrabMinSize = 10.0f;                  // Minimum width/height of a grab box for slider/scrollbar
    float GrabRounding = 0.0f;                  // Radius of grabs corners rounding
    
    float TabRounding = 4.0f;                   // Radius of upper corners of a tab
    float TabBorderSize = 0.0f;                 // Thickness of border around tabs
    
    Vec2 ButtonTextAlign = Vec2(0.5f, 0.5f);    // Alignment of button text
    Vec2 SelectableTextAlign = Vec2(0.0f, 0.0f);// Alignment of selectable text
    
    Vec2 DisplayWindowPadding = Vec2(19, 19);   // Window position are clamped to be visible within the display area
    Vec2 DisplaySafeAreaPadding = Vec2(3, 3);   // If you cannot see the edges of your screen
    
    float MouseCursorScale = 1.0f;              // Scale software rendered mouse cursor
    bool AntiAliasedLines = true;               // Enable anti-aliased lines/borders
    bool AntiAliasedFill = true;                // Enable anti-aliased filled shapes
    float CurveTessellationTol = 1.25f;         // Tessellation tolerance when using PathBezierCurveTo()
    
    // Colors
    std::array<Color, GUICol_COUNT> Colors;

    GUIStyle();
    
    // Helper functions
    void setColor(GUICol idx, const Color& color);
    Color getColor(GUICol idx) const;
    
    // Load default dark theme
    void setDarkTheme();
    
    // Load light theme
    void setLightTheme();
    
    // Load classic theme
    void setClassicTheme();
};

} // namespace CustomGUI
