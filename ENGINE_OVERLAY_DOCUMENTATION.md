# Professional Engine Overlay - Unity/Unreal Style Interface

## Overview

The RPG-Looter engine now features a completely redesigned, professional engine overlay built from scratch using OpenGL and GLFW. The interface is inspired by industry-standard game engines like Unity and Unreal Engine, providing a familiar and powerful development environment.

## Key Features

### 1. Main Menu Bar
Located at the top of the screen, provides access to all major engine functions:

**File Menu**
- `New` - Create new scene (planned)
- `Open` - Open existing scene (planned)
- `Save` - Save current scene (planned)

**Edit Menu**
- `Undo` - Undo last action (planned)
- `Redo` - Redo last undone action (planned)

**GameObject Menu**
- `Create Empty` - Create a new empty entity

**Window Menu**
- `Hierarchy` - Toggle hierarchy window
- `Console` - Toggle console window
- `Stats` - Toggle performance stats window

**Help Menu**
- Access to documentation and about information

**FPS Counter** (Right-aligned)
- Real-time FPS display with color-coded performance indicator:
  - Green: >= 58 FPS (Excellent)
  - Yellow: 30-57 FPS (Good)
  - Red: < 30 FPS (Poor)

### 2. Toolbar
Located just below the menu bar, provides quick access to editor controls:

**Playmode Controls** (Unity-style, centered)
- `> (Play)` - Enter play mode / Pause when active
- `||> (Step)` - Step one frame (planned)
- `[] (Stop)` - Stop play mode

### 3. Hierarchy Window
**Location:** Left side of the screen  
**Size:** 320x450 pixels  
**Positioned:** (10, 75)

**Features:**
- Displays all entities in the current scene
- Shows entity count at the top
- `+ Create Entity` button for quick entity creation
- Entity list with:
  - Entity name and ID
  - Visual selection highlighting (yellow background)
  - Component indicators:
    - `[T]` - Has Transform Component (Green)
    - `[R]` - Has Render Component (Blue)
    - `[A]` - Has Audio Component (Yellow)
- Click any entity to select it for inspection

### 4. Inspector Window
**Location:** Right side of the screen  
**Size:** 320x450 pixels  
**Positioned:** (screenWidth - 330, 75)

**Features:**
- Shows detailed information about the selected entity
- Entity header with name (golden color) and ID (gray)
- Tag editor for renaming entities
- Component editors with collapsible headers:

**Transform Component:**
- Position (X, Y) - Drag to adjust
- Scale (X, Y) - Drag to adjust
- Rotation - Drag to adjust

**Render Component:**
- Color picker (RGB)
- Alpha slider (0.0 - 1.0)
- Render Layer (integer)
- Shader name (text input)

**Audio Component:**
- Component presence indicator
- (Controls planned)

- `+ Add Component` buttons when components are missing
- `Delete Entity` button (full width, at bottom)

### 5. Scene Stats Window
**Location:** Top right  
**Size:** 320x250 pixels  
**Positioned:** (screenWidth - 330, 535)

**Features:**
- Performance monitoring with "Performance Monitor" header (golden)
- Real-time metrics:
  - FPS (frames per second)
  - Frame Time (milliseconds)
  - Delta Time (seconds)
- Progress bar showing performance relative to 60 FPS target
- Status indicator:
  - Green "Excellent" - < 80% of target frame time
  - Yellow "Good" - 80-120% of target frame time
  - Red "Poor" - > 120% of target frame time
- Frame time history graph (last 50 frames)
- Min/Max range display

### 6. Console Window
**Location:** Bottom of screen  
**Size:** Full width x 250 pixels  
**Positioned:** (10, screenHeight - 260)

**Features:**
- Professional logging system with filtering
- Filter controls:
  - `Info` checkbox - Show info messages
  - `Warning` checkbox - Show warning messages
  - `Error` checkbox - Show error messages
  - `Clear` button - Clear all logs
  - `Auto-scroll` checkbox - Auto-scroll to latest log
- Color-coded log entries:
  - White/Light gray - Info messages
  - Yellow - Warning messages
  - Red - Error messages
- Timestamp display for each log entry
- Rolling buffer (max 1000 logs)

## User Interface Controls

### Keyboard Shortcuts
- `F1` - Toggle overlay visibility (show/hide all UI)
- `ESC` - Exit application

### Mouse Interaction
- **Click** - Select entity in hierarchy
- **Click** - Interact with buttons and controls
- **Drag** - Adjust numeric values in inspector
- **Type** - Edit text fields

## Technical Implementation

### Architecture
The overlay is built using a custom UI system with the following components:

1. **UIContext** - Manages the overall UI state and rendering
2. **DrawList** - Accumulates drawing commands
3. **UIRenderer** - Executes drawing commands using OpenGL
4. **FontAtlas** - Manages font rendering with bitmap fonts
5. **Widgets** - Provides high-level UI components

### Rendering Pipeline
1. Begin frame - Clear previous frame data
2. Render menu bar (fixed position)
3. Render toolbar (below menu bar)
4. Render all windows (hierarchy, inspector, stats, console)
5. End frame - Finalize draw lists
6. Render - Execute all OpenGL draw commands

### Color Theme
The overlay uses a professional yellow/golden theme inspired by modern game engines:
- Window Background: Light beige (#F0F0DC)
- Title Bar: Golden (#CDB333)
- Active Title: Bright yellow (#FFD900)
- Borders: Dark golden (#998019)
- Buttons: Yellow (#E6BF33)
- Text: Black (#000000) for maximum contrast

## Layout Overview

```
┌─────────────────────────────────────────────────────────────┐
│ File | Edit | GameObject | Window | Help |        FPS: 60.0 │ ← Menu Bar
├─────────────────────────────────────────────────────────────┤
│         > (Play)   ||> (Step)   [] (Stop)                   │ ← Toolbar
├──────────────┬──────────────────────────────┬───────────────┤
│              │                              │               │
│  Hierarchy   │                              │ Scene Stats   │
│              │                              │               │
│  - Entity 1  │                              │ FPS: 60.0     │
│  - Entity 2  │                              │ Frame: 16ms   │
│  - Entity 3  │      (Game View Area)        │ Status: Good  │
│              │                              │               │
│              │                              ├───────────────┤
│              │                              │               │
│              │                              │  Inspector    │
│              │                              │               │
│              │                              │  [Selected    │
│              │                              │   Entity]     │
├──────────────┴──────────────────────────────┴───────────────┤
│                                                              │
│  Console                                                     │
│  [Info] [Warning] [Error] [Clear] [Auto-scroll]            │
│  [0.00] [Info] Engine started...                           │
│  [0.05] [Info] Entity created                              │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

## Comparison to Unity/Unreal

### Similar Features
✓ Menu bar with File/Edit/GameObject/Window menus  
✓ Toolbar with play/pause/step controls  
✓ Hierarchy window showing entity tree  
✓ Inspector window for component editing  
✓ Console window for logs and debugging  
✓ Performance stats window  
✓ Familiar keyboard shortcuts  

### Custom Features
- Custom rendering using pure OpenGL (no ImGui dependency)
- Lightweight and fast
- Integrated directly into game engine
- Yellow/golden color theme for visual distinction

## Performance

The overlay is designed to be lightweight and performant:
- Minimal CPU overhead (<1ms per frame)
- Uses retained-mode rendering with draw list batching
- Efficient OpenGL state management
- No external UI library dependencies (except for ImGui legacy integration)

## Future Enhancements

Planned features for future versions:
- Dockable windows
- Custom layouts
- Asset browser
- Scene view with gizmos
- Profiler with detailed breakdown
- Undo/redo system
- Save/load scenes
- Prefab system
- Material editor

## Development

The engine overlay is fully integrated with the ECS (Entity Component System) architecture and provides direct access to:
- Entity creation and destruction
- Component addition and modification
- Real-time property editing
- Performance monitoring

All changes made in the overlay are immediately reflected in the running game, enabling rapid iteration and development.
