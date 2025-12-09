# Custom GUI System - Usage Examples

This directory contains a custom Immediate-Mode GUI system similar to Dear ImGui.

## Basic Usage

```cpp
#include "CustomGUI/CustomGUI.h"

// Initialize (once at startup)
CustomGUI::GUIContext* ctx = CustomGUI::createContext();
CustomGUI::GUIRenderer renderer;
renderer.init();

// In your main loop:
while (running) {
    // Start new frame
    CustomGUI::newFrame();
    
    // Create a window
    if (CustomGUI::begin("Demo Window")) {
        // Add widgets
        CustomGUI::text("Hello, World!");
        CustomGUI::text("This is a custom GUI system");
        
        // Button
        if (CustomGUI::button("Click Me!")) {
            printf("Button was clicked!\n");
        }
        
        // Slider
        static float value = 0.5f;
        if (CustomGUI::sliderFloat("My Slider", &value, 0.0f, 1.0f)) {
            printf("Slider value: %.2f\n", value);
        }
        
        // Checkbox
        static bool enabled = true;
        CustomGUI::checkbox("Enable Feature", &enabled);
        
        // Color editor
        static float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
        CustomGUI::colorEdit4("My Color", color);
        
        // Layout
        CustomGUI::separator();
        CustomGUI::spacing();
        CustomGUI::text("Item 1");
        CustomGUI::sameLine();
        CustomGUI::text("Item 2");
    }
    CustomGUI::end();
    
    // End frame and render
    CustomGUI::endFrame();
    CustomGUI::render();
    renderer.render(CustomGUI::getCurrentContext()->getDrawList());
    
    // Swap buffers, etc.
}

// Cleanup
renderer.shutdown();
CustomGUI::destroyContext(ctx);
```

## Features

### Core Features (Implemented)
- ✅ Immediate-mode API
- ✅ Window management with Begin()/End()
- ✅ Text rendering
- ✅ Buttons
- ✅ Checkboxes and radio buttons
- ✅ Sliders (float and int)
- ✅ Color editors
- ✅ Layout controls (sameLine, spacing, separator, indent)
- ✅ Tree nodes
- ✅ Selectables
- ✅ Basic draw list with primitives
- ✅ OpenGL renderer with batching
- ✅ Multiple color themes (Dark, Light, Classic)

### Styling

```cpp
// Switch themes
CustomGUI::styleColorsDark();    // Dark theme (default)
CustomGUI::styleColorsLight();   // Light theme
CustomGUI::styleColorsClassic(); // Classic ImGui theme

// Customize individual colors
CustomGUI::GUIStyle& style = CustomGUI::getStyle();
style.setColor(CustomGUI::GUICol_Button, CustomGUI::Color(0.2f, 0.4f, 0.8f, 1.0f));
style.setColor(CustomGUI::GUICol_ButtonHovered, CustomGUI::Color(0.3f, 0.5f, 0.9f, 1.0f));

// Adjust spacing and sizes
style.WindowPadding = CustomGUI::Vec2(10, 10);
style.FramePadding = CustomGUI::Vec2(5, 4);
style.ItemSpacing = CustomGUI::Vec2(10, 5);
style.FrameRounding = 3.0f;
```

### Drawing Primitives

```cpp
CustomGUI::GUIDrawList* drawList = CustomGUI::getWindowDrawList();

// Draw shapes
drawList->addLine(CustomGUI::Vec2(10, 10), CustomGUI::Vec2(100, 100), 
                  CustomGUI::Color(1, 0, 0, 1), 2.0f);
                  
drawList->addRect(CustomGUI::Vec2(50, 50), CustomGUI::Vec2(150, 150), 
                  CustomGUI::Color(0, 1, 0, 1), 5.0f, 2.0f);
                  
drawList->addRectFilled(CustomGUI::Vec2(200, 50), CustomGUI::Vec2(300, 150), 
                        CustomGUI::Color(0, 0, 1, 1), 5.0f);
                        
drawList->addCircle(CustomGUI::Vec2(400, 100), 50.0f, 
                    CustomGUI::Color(1, 1, 0, 1), 32, 2.0f);
                    
drawList->addCircleFilled(CustomGUI::Vec2(500, 100), 50.0f, 
                          CustomGUI::Color(1, 0, 1, 1), 32);
```

### Multiple Windows

```cpp
// Window 1
if (CustomGUI::begin("Window 1")) {
    CustomGUI::text("This is window 1");
    CustomGUI::button("Button 1");
}
CustomGUI::end();

// Window 2
if (CustomGUI::begin("Window 2")) {
    CustomGUI::text("This is window 2");
    CustomGUI::button("Button 2");
}
CustomGUI::end();

// Window with close button
static bool showWindow3 = true;
if (CustomGUI::begin("Window 3", &showWindow3)) {
    CustomGUI::text("This window can be closed");
}
CustomGUI::end();
```

## Architecture

The Custom GUI system consists of these main components:

1. **GUIContext** - Central state management
   - Manages windows, active/hovered IDs
   - Handles input state
   - Controls layout cursor

2. **GUIStyle** - Visual styling
   - Colors for all UI elements
   - Spacing, padding, rounding
   - Multiple built-in themes

3. **GUIDrawList** - Command buffer
   - Accumulates vertices and indices
   - Batches draw calls
   - Supports primitive shapes

4. **GUIRenderer** - OpenGL backend
   - Renders draw lists efficiently
   - Handles textures and clipping
   - State management

5. **CustomGUI.h** - Public API
   - High-level widget functions
   - Immediate-mode interface
   - Compatible with ImGui patterns

## Input Integration

To integrate with your input system:

```cpp
// Mouse input
CustomGUI::setMousePos(mouseX, mouseY);
CustomGUI::setMouseButton(0, leftButtonPressed);
CustomGUI::setMouseButton(1, rightButtonPressed);

// Display size
CustomGUI::setDisplaySize(windowWidth, windowHeight);
```

## Performance

The system uses:
- Vertex buffer batching for efficient rendering
- Minimal state changes
- Scissor rectangles for clipping
- Single draw call per texture/clip rect combination

## Differences from ImGui

This is a simplified implementation focused on core features:
- No font atlas system (simplified text rendering)
- No docking or viewports
- Simplified input handling
- Basic text input support
- No tables or advanced layout

## Future Enhancements

Possible improvements:
- TrueType font rendering with SDF
- Full keyboard input support
- Tables and advanced layouts
- Docking system
- Multi-viewport support
- More widget types
- Better text rendering
- Input text with selection
