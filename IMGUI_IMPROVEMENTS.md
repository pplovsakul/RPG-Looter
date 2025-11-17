# ImGui Windows Improvements - User Guide

## Overview

This update significantly enhances the ImGui-based editor interface with new windows, improved functionality, and better user experience. The editor now provides a professional, feature-rich environment for game development.

## New Windows

### 1. Performance Monitor (Replaces Basic Debug Window)
**Default Position:** Top-left corner

**Features:**
- Real-time FPS and frame time display
- Performance graphs showing last 2 seconds of data
- Detailed statistics (min/max/average frame times)
- Entity count with component breakdown
- Collapsible sections for better organization
- System toggle controls (placeholder)

**How to Use:**
- View real-time performance metrics
- Monitor entity count as you add/remove objects
- Check component distribution across entities
- Use for performance optimization

### 2. Console Window
**Default Position:** Bottom-left area

**Features:**
- Centralized logging with four levels (Info, Warning, Error, Debug)
- Color-coded messages for easy identification
- Timestamps (HH:MM:SS.mmm format)
- Filter logs by level using checkboxes
- Auto-scroll to latest messages
- Command input (prepared for future command system)
- Stores up to 1000 log entries

**How to Use:**
```cpp
// In your code, use the static methods:
ConsoleWindow::Info("Application started");
ConsoleWindow::Warning("Low memory");
ConsoleWindow::Error("Failed to load texture");
ConsoleWindow::Debug("Variable value: " + std::to_string(x));
```

### 3. Scene Hierarchy Window
**Default Position:** Left side, middle

**Features:**
- Tree view of all entities
- Search/filter entities by name
- Group by tag option
- Component indicators: [T]ransform, [R]ender, [A]udio, [M]odel
- Right-click context menu for quick actions
- Entity duplication with component copying
- Quick component add/remove

**How to Use:**
- Search: Type in the search box to filter entities
- Group by Tag: Enable to organize entities by their tags
- Right-click: Access quick actions (Select, Duplicate, Delete, Add/Remove components)
- Click: Select an entity (integrates with Entity Editor)

### 4. Settings Window
**Default Position:** Not shown by default

**Tabs:**
- **Windows:** Toggle visibility of editor windows (placeholder)
- **Rendering:** VSync, FPS target, graphics quality (placeholders)
- **Editor:** Auto-save settings, default paths
- **UI:** Scale (0.5x-2.0x), color theme (Dark/Light/Classic)

**How to Use:**
- Adjust UI scale for better visibility
- Change color theme to match your preference
- Configure default save paths
- Access ImGui demo window for reference

### 5. Quick Actions & Help
**Location:** Main menu bar at the top

**Menus:**
- **File:** New/Save/Load scene, Exit
- **Edit:** Undo/Redo (placeholders), Copy/Paste
- **Entity:** Create Empty/Sprite, Delete All
- **View:** Toggle windows (F1-F6)
- **Help:** Keyboard shortcuts (F12), About

**Quick Actions Window:** Floating window with one-click actions

**How to Use:**
- Press F12 to view all keyboard shortcuts
- Use menu items for quick entity creation
- Check "Getting Started" tab for onboarding
- Access About page for technology info

## Enhanced Windows

### Entity Editor (Improved)

**New Features:**
- Tabbed interface: Edit, Templates, Batch, Save/Load
- Search/filter entities
- Component indicators in list view
- Quick entity templates
- Component copy/paste between entities
- Right-click context menu
- Batch operations

**Templates:**
- Empty Entity
- Sprite Entity (Transform + Render)
- Audio Source (Transform + Audio)
- Complete Entity (Transform + Render + Audio)

**Batch Operations:**
- Delete All Entities (with confirmation)
- Delete Entities Without Transform
- Add Transform to All Entities

**How to Use Copy/Paste:**
1. Select an entity
2. Click "Copy Components"
3. Select another entity
4. Click "Paste Components"

### Asset Manager (Improved)

**New Features:**
- Tabbed interface: Textures, Sounds, Models, Statistics, Stress Test
- Search/filter for all asset types
- Adjustable texture preview size (32-128px)
- Hover tooltips with larger previews
- Asset statistics with memory estimation
- Two-column model display (Textured vs Plain)
- Entity count tracking in stress tests

**Tabs:**
- **Textures:** Browse, preview, load textures
- **Sounds:** Browse, load sounds (play controls placeholder)
- **Models:** View textured and plain models separately
- **Statistics:** View total assets, memory usage, detailed breakdowns
- **Stress Test:** CPU and GPU performance testing

**How to Use:**
- Adjust preview size with the slider
- Hover over textures for larger preview
- Use search to find specific assets
- Check Statistics tab for memory usage
- Use Stress Test to benchmark performance

### Model Editor (Enhanced)

**New Features:**
- Grid visualization with major/minor lines
- Snap to grid functionality
- Adjustable grid size (5-100 units)
- Center crosshair for reference
- Toolbar with model management
- View controls (Show Grid, Snap to Grid)
- Confirmation dialogs

**How to Use:**
- Enable "Show Grid" to see the grid overlay
- Enable "Snap to Grid" for precise positioning
- Adjust "Grid Size" to change snap granularity
- Use toolbar buttons for New/Clear operations
- Drag shapes in the preview to move them
- Grid helps align shapes perfectly

## Keyboard Shortcuts

### General
- `ESC` - Exit application
- `F12` - Show/Hide help window

### File Operations
- `Ctrl+N` - New scene (placeholder)
- `Ctrl+S` - Save scene (placeholder)
- `Ctrl+O` - Load scene (placeholder)

### Entity Operations
- `Ctrl+Shift+N` - Create empty entity
- `Ctrl+Shift+S` - Create sprite entity
- `Del` - Delete selected entity (placeholder)

### Window Toggles
- `F1` - Performance Monitor (placeholder)
- `F2` - Console (placeholder)
- `F3` - Scene Hierarchy (placeholder)
- `F4` - Entity Editor (placeholder)
- `F5` - Asset Manager (placeholder)
- `F6` - Model Editor (placeholder)

**Note:** Window toggle shortcuts are documented but not yet functional. They will be implemented in a future update.

## Tips & Best Practices

### Organization
1. Use tags to group related entities
2. Enable "Group by Tag" in Scene Hierarchy for better organization
3. Use component indicators to quickly identify entity types

### Workflow
1. Create entities using templates in Entity Editor
2. Assign models from Model Editor
3. Load textures through Asset Manager
4. Monitor performance with Performance Window
5. Check Console for any issues

### Performance
1. Use the Stress Test to identify performance limits
2. Monitor entity count in Performance Window
3. Check detailed statistics in Asset Manager
4. Keep an eye on frame time graphs

### Asset Management
1. Organize assets in subfolders (res/textures/, res/sounds/, etc.)
2. Use descriptive names for easy searching
3. Check Statistics tab to monitor memory usage
4. Unload unused assets to free memory

## Future Enhancements

The following features are prepared but not yet implemented:

### Short Term
- Window visibility toggles (F1-F6 shortcuts)
- Actual VSync control
- Undo/Redo system
- Scene save/load functionality
- Zoom and pan in Model Editor
- Sound playback controls in Asset Manager

### Medium Term
- Command system in Console
- Auto-save functionality
- Entity hierarchy (parent-child relationships)
- Asset hot-reloading
- Custom keyboard shortcuts
- Dockable windows

### Long Term
- Visual scripting system
- Animation editor
- Particle system editor
- Tilemap editor
- Level editor

## Troubleshooting

### Window Not Showing
- Check if it's off-screen (reset positions in Settings)
- Look for it in the View menu
- Restart the application

### Performance Issues
- Reduce entity count
- Check Stress Test results
- Monitor Performance Window for bottlenecks
- Disable unused systems

### Console Spam
- Use log level filters to hide debug messages
- Clear console regularly
- Check for error messages

## Architecture Notes

All windows are implemented as Systems in the ECS architecture:
- Inherit from `System` base class
- Implement `update(EntityManager&, float)` method
- Registered in `Game::setupSystems()`
- Updated every frame in render loop

This allows:
- Easy addition of new windows
- Consistent update pattern
- Access to entity manager
- Integration with game loop

## Summary

This update adds **5 new windows** and **enhances 3 existing windows**, providing:
- Better performance monitoring
- Centralized logging
- Improved entity management
- Professional asset management
- Enhanced model editing
- Global settings
- Quick actions and help

The editor is now a powerful, professional tool for 2D game development!
