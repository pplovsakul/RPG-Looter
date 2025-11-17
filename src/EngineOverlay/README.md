# Engine Overlay - Custom UI System

A custom, lightweight UI overlay system for the RPG Looter engine, built from scratch using OpenGL.

## Features

### Core System
- **Custom rendering pipeline** with proper OpenGL state management
- **Bitmap font atlas** with readable characters (5x7 pixel patterns)
- **Proper blending** using `glBlendFuncSeparate` for correct alpha handling
- **Scissor-based clipping** for window content areas
- **Dark theme** optimized for development/debugging

### Windows

#### Demo Window
Shows all available widgets and their usage including text, buttons, checkboxes, sliders, and more.

#### Entity Inspector
Real-time entity management with create/destroy, component display, and filtering.

#### Profiler
Performance monitoring with FPS, frame time, and history graph.

#### Console
Rolling log system with filtering, timestamps, and color-coded levels.

#### Debug UI
Render state diagnostics showing OpenGL state, font atlas info, and input state.

### Controls

- **F1**: Toggle overlay visibility
- **Mouse**: Click and interact with widgets
- **Menu Bar**: Quick access to show/hide windows

## Troubleshooting

### White Boxes / No Text Visible

**Recent Fixes Applied**:
1. Shader now always samples texture (no conditional logic)
2. BlendFuncSeparate configured properly
3. Font atlas uses readable 5x7 patterns
4. UVs fixed to (0.5, 0.5) for solid colors
5. Window titles render correctly
6. **UV Y-coordinates flipped** for proper OpenGL texture mapping (fixes garbled text)

Check Debug UI window for real-time diagnostics.

### Garbled/Upside-down Text

If text appears garbled or incorrect, this was caused by UV coordinate mismatch between bitmap creation (Y=0 at top) and OpenGL texture coordinates (Y=0 at bottom). This has been fixed in `FontAtlas.cpp` by properly flipping Y-coordinates using the formula `1.0 - y` during glyph UV calculation.

## License

Part of the RPG-Looter project.
