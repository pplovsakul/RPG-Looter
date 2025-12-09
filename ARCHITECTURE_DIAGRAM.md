# Custom GUI System Architecture

## Component Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                          Application Layer                           │
│  (Game, Editor, Debug Tools)                                        │
└──────────────────────────┬──────────────────────────────────────────┘
                           │
                           │ uses CustomGUI API
                           ▼
┌─────────────────────────────────────────────────────────────────────┐
│                         CustomGUI.h/cpp                              │
│  ┌────────────────────────────────────────────────────────────┐    │
│  │ Public API - Immediate Mode Interface                       │    │
│  │                                                              │    │
│  │  • begin()/end()        - Window management                 │    │
│  │  • text()               - Text rendering                    │    │
│  │  • button()             - Interactive buttons               │    │
│  │  • sliderFloat/Int()    - Value sliders                    │    │
│  │  • checkbox()           - Boolean toggles                   │    │
│  │  • colorEdit3/4()       - Color pickers                    │    │
│  │  • radioButton()        - Option selection                  │    │
│  │  • treeNode()           - Hierarchical data                │    │
│  │  • sameLine()           - Layout control                    │    │
│  │  • separator()          - Visual separation                │    │
│  │  • styleColors*()       - Theme switching                   │    │
│  └────────────────────────────────────────────────────────────┘    │
└────────────────┬───────────────────────────┬────────────────────────┘
                 │                           │
                 │ uses                      │ uses
                 ▼                           ▼
┌────────────────────────────┐   ┌──────────────────────────────┐
│     GUIContext.h/cpp       │   │     GUIStyle.h/cpp           │
│ ┌────────────────────────┐ │   │ ┌──────────────────────────┐ │
│ │  State Management      │ │   │ │  Visual Configuration    │ │
│ │                        │ │   │ │                          │ │
│ │  • Window Stack        │ │   │ │  • Colors (46 slots)     │ │
│ │  • Active/Hovered IDs  │ │   │ │  • Spacing & Padding     │ │
│ │  • Input State         │ │   │ │  • Rounding & Borders    │ │
│ │  • Layout Cursor       │ │   │ │  • Theme Presets         │ │
│ │  • ID Generation       │ │   │ │    - Dark (default)      │ │
│ │  • Frame Counter       │ │   │ │    - Light               │ │
│ │                        │ │   │ │    - Classic             │ │
│ └────────────────────────┘ │   │ └──────────────────────────┘ │
└────────────┬───────────────┘   └──────────────────────────────┘
             │
             │ generates
             ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      GUIDrawList.h/cpp                               │
│  ┌────────────────────────────────────────────────────────────┐    │
│  │ Command Buffer - Accumulates Rendering Data                │    │
│  │                                                              │    │
│  │  Vertices   [pos, uv, color, pos, uv, color, ...]          │    │
│  │  Indices    [0, 1, 2, 0, 2, 3, ...]                        │    │
│  │  Commands   [{count, clip, tex}, {count, clip, tex}, ...]  │    │
│  │                                                              │    │
│  │  Primitives:                                                │    │
│  │  • addRect / addRectFilled                                 │    │
│  │  • addCircle / addCircleFilled                             │    │
│  │  • addLine / addTriangle                                   │    │
│  │  • addText / addImage                                      │    │
│  │  • pathLineTo / pathArcTo / pathFillConvex                 │    │
│  └────────────────────────────────────────────────────────────┘    │
└────────────────────────────┬────────────────────────────────────────┘
                             │
                             │ consumed by
                             ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      GUIRenderer.h/cpp                               │
│  ┌────────────────────────────────────────────────────────────┐    │
│  │ OpenGL 3.3 Backend - Renders Draw Commands                 │    │
│  │                                                              │    │
│  │  Shader Program:                                            │    │
│  │    Vertex Shader   - Transform vertices with projection    │    │
│  │    Fragment Shader - Apply colors and textures             │    │
│  │                                                              │    │
│  │  Buffers:                                                   │    │
│  │    VAO - Vertex Array Object                               │    │
│  │    VBO - Vertex Buffer (dynamic)                           │    │
│  │    EBO - Element Buffer (dynamic)                          │    │
│  │                                                              │    │
│  │  Features:                                                  │    │
│  │    • Batched rendering (minimize draw calls)               │    │
│  │    • Scissor clipping (efficient culling)                  │    │
│  │    • State preservation (no GL state pollution)            │    │
│  │    • Texture binding per command                           │    │
│  └────────────────────────────────────────────────────────────┘    │
└────────────────────────────┬────────────────────────────────────────┘
                             │
                             │ renders to
                             ▼
┌─────────────────────────────────────────────────────────────────────┐
│                        OpenGL Framebuffer                            │
│                    (Screen or Render Target)                         │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│                          GUITypes.h                                  │
│  Foundation Types & Enums (used by all components)                  │
│                                                                      │
│  • Vec2, Vec4, Color, ID, Rect                                      │
│  • GUICol (46 color indices)                                        │
│  • GUIWindowFlags, GUIMouseButton, GUIKey                           │
│  • GUIStyleVar, GUIItemFlags                                        │
└─────────────────────────────────────────────────────────────────────┘
```

## Data Flow

```
Frame Start
    │
    ▼
newFrame()
    │
    ├─ Clear draw list
    ├─ Update input state
    └─ Reset frame state
    │
    ▼
begin("Window")
    │
    ├─ Find or create window
    ├─ Update window layout
    ├─ Push to window stack
    └─ Initialize cursor position
    │
    ▼
widget() calls
    │
    ├─ Generate ID (hash)
    ├─ Check hover/active state
    ├─ Process input
    ├─ Add draw commands to list
    └─ Update cursor position
    │
    ▼
end()
    │
    ├─ Pop window stack
    └─ Finalize window
    │
    ▼
endFrame()
    │
    └─ Finalize state
    │
    ▼
render()
    │
    └─ Render all windows
    │
    ▼
renderer.render(drawList)
    │
    ├─ Upload vertices to VBO
    ├─ Upload indices to EBO
    ├─ For each command:
    │   ├─ Set scissor rect
    │   ├─ Bind texture
    │   └─ glDrawElements()
    │
    ▼
GPU Rendering
    │
    └─ Display on screen
```

## ID Generation Flow

```
User Code: button("MyButton")
    │
    ▼
getID("MyButton")
    │
    ├─ Get seed from ID stack (window/scope)
    │
    ▼
FNV-1a Hash
    │
    ├─ hash = 0x811c9dc5
    ├─ For each char: hash = (hash ^ char) * 0x01000193
    │
    ▼
Combine with seed
    │
    └─ finalID = seed ^ hash
    │
    ▼
Use ID for state tracking
    │
    ├─ Check if ID == hoveredId
    ├─ Check if ID == activeId
    └─ Store interaction state
```

## Widget Lifecycle

```
┌────────────────────┐
│   Frame N-1        │
│   Widget State     │
│   (in context)     │
└─────────┬──────────┘
          │
          ▼
    ┌─────────────────────┐
    │   Frame N Start     │
    │   newFrame()        │
    └──────────┬──────────┘
               │
               ▼
    ┌──────────────────────┐
    │   Widget Call        │
    │   button("OK")       │
    └──────────┬───────────┘
               │
               ├─── Generate ID
               │
               ├─── Check mouse hover
               │        │
               │        ├─ Inside bounds? → Set hoveredId
               │        └─ Outside? → Clear hoveredId
               │
               ├─── Check mouse click
               │        │
               │        ├─ Clicked while hovered? → Set activeId
               │        └─ Released while active? → Return true
               │
               ├─── Render button
               │        │
               │        ├─ Choose color (normal/hover/active)
               │        ├─ Add rectangle to draw list
               │        └─ Add text to draw list
               │
               └─── Update cursor
                        │
                        └─ Move to next widget position
               │
               ▼
    ┌──────────────────────┐
    │   End Frame          │
    │   endFrame()         │
    └──────────┬───────────┘
               │
               ▼
    ┌──────────────────────┐
    │   Render             │
    │   renderer.render()  │
    └──────────────────────┘
```

## Memory Layout

```
GUIContext
├── windows: vector<GUIWindow*>
│   └── Each window:
│       ├── name: string
│       ├── pos, size: Vec2
│       ├── flags: int
│       └── drawList: GUIDrawList*
│
├── drawList: GUIDrawList (main)
│   ├── vertices: vector<GUIVertex>
│   │   └── [pos.x, pos.y, uv.x, uv.y, r, g, b, a] × N
│   ├── indices: vector<uint32>
│   │   └── [0, 1, 2, 3, ...] triangle indices
│   └── commands: vector<GUIDrawCmd>
│       └── [{elemCount, clipRect, textureId}, ...]
│
├── input: GUIInputState
│   ├── mousePos: Vec2
│   ├── mouseDown[3]: bool
│   ├── keysDown[KEY_COUNT]: bool
│   └── inputCharacters: string
│
├── style: GUIStyle
│   ├── Colors[46]: Color
│   ├── WindowPadding: Vec2
│   ├── FramePadding: Vec2
│   └── ...
│
└── state:
    ├── hoveredId: ID (uint32)
    ├── activeId: ID (uint32)
    ├── focusedId: ID (uint32)
    └── cursor: LayoutCursor
        ├── pos: Vec2
        ├── lineHeight: float
        └── indentX: float
```
