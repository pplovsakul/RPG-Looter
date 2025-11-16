# Engine Overlay System

A custom immediate-mode UI system built from scratch for the RPG-Looter game engine, providing an in-game overlay with professional game engine aesthetics.

## Overview

The Engine Overlay system is a **completely custom UI framework** that does not depend on ImGui or any other third-party UI library. It features:

- **Custom OpenGL renderer** with batched quad rendering
- **Immediate-mode API** for easy widget creation
- **Professional game engine aesthetics** (dark theme, crisp borders)
- **Window management** with clipping and layering
- **Bitmap font atlas** for text rendering
- **Input handling** with mouse and keyboard support
- **Toggle-able overlay** (F1 key by default)

## Architecture

### Core Components

1. **DrawList** (`DrawList.h/cpp`)
   - Accumulates rendering commands (vertices, indices, draw commands)
   - Primitive shapes: rectangles, lines, circles, triangles
   - Clipping rectangle support
   - Batched rendering for performance

2. **UIRenderer** (`UIRenderer.h/cpp`)
   - OpenGL-based renderer
   - Custom GLSL shaders for UI rendering
   - Handles texture binding, scissor testing, blending
   - Orthographic projection for 2D UI

3. **FontAtlas** (`FontAtlas.h/cpp`)
   - Bitmap font texture atlas (512x512)
   - Glyph information (UV coordinates, advance, bearing)
   - Text measurement utilities
   - Supports printable ASCII characters (32-126)

4. **UIContext** (`UIContext.h/cpp`)
   - Central UI state management
   - Window management and rendering order
   - Input state tracking and event handling
   - Widget ID generation and hot/active tracking
   - Layout system (cursor-based auto-layout)

5. **Widgets** (`Widgets.h/cpp`)
   - Immediate-mode widget API
   - Text, buttons, checkboxes, radio buttons
   - Sliders (int/float), drag widgets
   - Input text, progress bars, separators
   - Color editors, layout helpers

6. **EngineOverlaySystem** (`EngineOverlaySystem.h/cpp`)
   - ECS System integration
   - Game loop integration
   - Profiler HUD, Entity Inspector, Demo Window
   - F1 toggle functionality

## Usage

### Basic Window

```cpp
using namespace EngineUI;

// In your update loop:
if (ctx->beginWindow("My Window")) {
    Text("Hello, World!");
    
    if (Button("Click Me")) {
        std::cout << "Button clicked!" << std::endl;
    }
    
    static bool checkbox = false;
    Checkbox("Option", &checkbox);
    
    static float slider = 0.5f;
    SliderFloat("Value", &slider, 0.0f, 1.0f);
    
    ctx->endWindow();
}
```

### Widget API

#### Text
```cpp
Text("Plain text");
TextColored(Color(1, 0, 0, 1), "Red text");
```

#### Buttons
```cpp
if (Button("Click Me")) {
    // Button was clicked
}

if (Button("Custom Size", 200.0f, 50.0f)) {
    // Button with specific size
}
```

#### Checkboxes & Radio Buttons
```cpp
static bool checked = false;
Checkbox("Checkbox Label", &checked);

static int radioValue = 0;
if (RadioButton("Option 1", radioValue == 0)) radioValue = 0;
if (RadioButton("Option 2", radioValue == 1)) radioValue = 1;
```

#### Sliders
```cpp
static float floatVal = 0.5f;
SliderFloat("Float", &floatVal, 0.0f, 1.0f);

static int intVal = 50;
SliderInt("Integer", &intVal, 0, 100);
```

#### Input Text
```cpp
static char buffer[256] = "Edit me";
if (InputText("Text", buffer, sizeof(buffer))) {
    // Text was modified
}
```

#### Progress Bar
```cpp
float progress = 0.75f; // 0.0 to 1.0
ProgressBar(progress);
```

#### Layout
```cpp
Text("First");
SameLine(); // Next widget on same line
Text("Second");

Separator(); // Horizontal line

Spacing(); // Add vertical space

Indent();
Text("Indented");
Unindent();
```

### Style Customization

```cpp
Style& style = ctx->getStyle();

// Colors
style.windowBg = Color(0.1f, 0.1f, 0.1f, 0.95f);
style.button = Color(0.26f, 0.59f, 0.98f, 0.40f);
style.text = Color(1.0f, 1.0f, 1.0f, 1.0f);

// Spacing
style.windowPadding = 8.0f;
style.itemSpacing = 4.0f;

// Rounding
style.frameRounding = 2.0f;
```

## Integration

The system is integrated into the game as an ECS System:

```cpp
// In Game::setupSystems():
systems.push_back(std::make_unique<EngineOverlaySystem>(window));
```

The overlay automatically:
- Renders on top of the game
- Handles input via GLFW
- Toggles with F1 key
- Shows Entity Inspector, Profiler, and Demo windows

## Features

### Current Features
- ✅ Custom OpenGL batched renderer
- ✅ Immediate-mode widget API
- ✅ Window management with clipping
- ✅ Text rendering with bitmap font atlas
- ✅ Basic widgets (buttons, checkboxes, sliders, etc.)
- ✅ Layout system (SameLine, Spacing, Indent)
- ✅ Input handling (mouse, keyboard)
- ✅ F1 toggle
- ✅ Entity Inspector
- ✅ Profiler HUD
- ✅ Demo Window

### Planned Features
- ⏳ Window dragging and resizing
- ⏳ Docking system
- ⏳ Tab bars
- ⏳ Tree views and collapsing headers
- ⏳ Scrollable regions
- ⏳ Tables
- ⏳ Menu bars and popups
- ⏳ Tooltips
- ⏳ Advanced color picker
- ⏳ Vector editors (vec2/3/4)
- ⏳ Curve editor
- ⏳ Log console with filtering
- ⏳ Configuration persistence (JSON)
- ⏳ Theme system
- ⏳ Better font rendering (SDF or TrueType)

## Performance

The system is designed for minimal overhead:
- **Batched rendering**: All UI drawn in a single draw call per window
- **Object pooling**: Reuses vertex/index buffers
- **Immediate-mode**: No retained UI state, minimal memory
- **Simple shaders**: Optimized for 2D UI rendering

## Controls

- **F1**: Toggle overlay visibility
- **Mouse**: Interact with widgets
- **Click**: Activate buttons, checkboxes, etc.
- **Drag**: Adjust sliders
- **Type**: Edit text input fields

## Technical Details

### Rendering Pipeline
1. Begin frame: Reset input state, clear render lists
2. Widget calls: Add primitives to DrawList
3. End frame: Update hover state
4. Render: Execute all DrawLists with OpenGL

### Coordinate System
- Origin (0,0) is top-left
- Orthographic projection
- Y-axis points down

### Text Rendering
- 8x16 pixel monospace bitmap font
- 512x512 texture atlas
- Supports ASCII 32-126
- Simple pattern-based glyphs (can be replaced with real font)

## Files

```
src/EngineOverlay/
├── DrawList.h/cpp          # Rendering command accumulator
├── UIRenderer.h/cpp        # OpenGL renderer
├── FontAtlas.h/cpp         # Bitmap font system
├── UIContext.h/cpp         # Core UI state management
├── Widgets.h/cpp           # Widget implementations
└── EngineOverlaySystem.h/cpp # ECS integration
```

## Building

The overlay system is automatically included in the main project build:

```bash
cd build
cmake ..
make
```

No additional dependencies required - uses only OpenGL 3.3+ and GLFW.

## License

Part of the RPG-Looter project.

## Future Improvements

1. **Better Fonts**: Replace bitmap font with SDF or TrueType rendering
2. **Docking**: Add full docking system like modern IDEs
3. **Persistence**: Save/load layout and settings to JSON
4. **Themes**: Support multiple color themes
5. **Accessibility**: Keyboard navigation, screen reader support
6. **Performance**: More aggressive batching, GPU instancing
7. **Advanced Widgets**: Node editors, timeline, property grids
8. **Documentation**: More examples and tutorials

---

For questions or issues, please refer to the main RPG-Looter documentation.
