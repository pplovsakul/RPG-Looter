# 3D Model Visibility Improvements - Testing Guide

## Overview
This document provides testing instructions for the newly implemented visibility improvements in the RPG-Looter 3D game engine.

## What Was Implemented

### 1. White Background for Better Visibility
**Location**: Viewport/Rendering area
**What to test**: 
- Load a 3D model (OBJ file)
- Verify the background is now white instead of dark blue
- Check that models with light colors are now clearly visible

### 2. Focus Entity Feature
**Locations**: 
- Scene Hierarchy window (F3) - Bottom panel and right-click menu
- Entity Editor (F4) - Quick actions section

**What to test**:
1. Open Scene Hierarchy (F3)
2. Select an entity that has a TransformComponent
3. Click "Focus on Entity" button (or right-click → "Focus on Entity")
4. Verify camera automatically moves to view the entity
5. Try with entities of different sizes/scales
6. Repeat from Entity Editor using "Focus Camera" button

**Expected behavior**:
- Camera positions itself behind and above the entity
- Camera automatically rotates to look at the entity
- Distance adjusts based on entity scale (larger = further back)
- Minimum distance is 5 units

### 3. Keyboard Shortcuts (F1-F6)
**What to test**:
- Press F1 → Performance Monitor toggles
- Press F2 → Console Window toggles
- Press F3 → Scene Hierarchy toggles
- Press F4 → Entity Editor toggles
- Press F5 → Asset Manager toggles
- Press F6 → Model Editor toggles (NEW!)

**Expected behavior**:
- Each key press toggles the corresponding window
- Holding the key doesn't cause rapid toggling
- All shortcuts work from any window/state

### 4. Model Editor Window (F6)
**What to test**:
1. Load one or more 3D models (OBJ/MTL files)
2. Press F6 to open Model Editor
3. Check the model list on the left
4. Select a model from the list
5. Verify the details panel shows:
   - Overview: Number of meshes, total vertices/triangles
   - Mesh Details: Per-mesh statistics (vertices, normals, UVs, indices)
   - Material/Texture info for each mesh
   - Bounding box calculations (min, max, size, center)
   - Data Quality checks and warnings

**Features to verify**:
- Search bar filters models by name
- Mesh count matches expectations
- Vertex/triangle counts are accurate
- Material names displayed correctly
- Texture names shown (if present)
- Color preview displays mesh colors
- Warnings appear for missing data (normals, UVs, etc.)

### 5. Enhanced Entity Editor
**What to test**:
1. Open Entity Editor (F4)
2. Select an entity with a ModelComponent
3. Expand the "Model" section
4. Verify you see:
   - Green text: "Entity has ModelComponent assigned"
   - Number of Meshes
   - Total Vertices and Triangles
   - Mesh Details tree (expandable)
     - Per-mesh vertices/triangles
     - Material name
     - Texture name
     - Color (editable with color picker)

**Expected behavior**:
- Statistics update when model is changed
- Color editing works for individual meshes
- All mesh information is accurate

### 6. Debug Output
**What to test**:
1. Open a terminal/console to see application output
2. Create or load entities with ModelComponent
3. Watch for console messages like:
   ```
   [RenderSystem] Now rendering 2 entities with ModelComponent
     - Entity [5] Player1: 3 meshes
     - Entity [8] Cube: 1 meshes
   ```

**Expected behavior**:
- Debug messages appear only when entity count changes
- Messages show entity ID, tag, and mesh count
- No messages appear every frame (performance optimization)
- In release builds (NDEBUG defined), no debug output

## Testing Checklist

### Basic Functionality
- [ ] Application builds and runs
- [ ] White background is visible
- [ ] Can load OBJ models via Asset Manager (F5)
- [ ] All F-key shortcuts work (F1-F6)
- [ ] Model Editor opens and displays loaded models

### Focus Entity Feature
- [ ] Focus button appears in Scene Hierarchy
- [ ] Focus button appears in Entity Editor
- [ ] Right-click menu has "Focus on Entity" option
- [ ] Camera moves when focus is activated
- [ ] Camera points at selected entity
- [ ] Works with entities of different sizes
- [ ] Minimum distance of 5 units is enforced

### Model Editor Window
- [ ] Window opens with F6
- [ ] Model list shows all loaded models
- [ ] Search filter works correctly
- [ ] Overview statistics are accurate
- [ ] Mesh details expand/collapse correctly
- [ ] Material information displays
- [ ] Texture information displays
- [ ] Bounding box calculations shown
- [ ] Data quality warnings appear when appropriate

### Entity Editor Enhancements
- [ ] ModelComponent section shows when entity has model
- [ ] Mesh count displayed correctly
- [ ] Vertex/triangle counts accurate
- [ ] Can expand mesh details
- [ ] Color editing works
- [ ] Material/texture info shown per mesh

### Performance
- [ ] No lag when toggling windows
- [ ] No lag when focusing on entities
- [ ] Model Editor opens quickly
- [ ] Debug output doesn't impact framerate
- [ ] UI remains responsive with many models loaded

## Known Limitations
1. Model Editor 3D preview is not yet implemented (placeholder text shown)
2. Focus camera does not animate (instant position change)
3. Debug output is only available in debug builds

## Troubleshooting

### Models Not Visible
1. Check if background is white (visibility improvement applied)
2. Use Focus Entity to position camera correctly
3. Open Model Editor (F6) to verify model loaded correctly
4. Check Entity Editor to confirm ModelComponent is assigned

### Camera Focus Not Working
1. Ensure entity has TransformComponent
2. Verify a camera exists (tagged "MainCamera" or has active CameraComponent)
3. Check entity scale is reasonable (not zero)

### Keyboard Shortcuts Not Working
1. Verify no modal dialog is open
2. Check if another application has keyboard focus
3. Try clicking the viewport/window first

### No Debug Output
1. Check if running a debug build (release builds have NDEBUG defined)
2. Verify entities with ModelComponent exist
3. Try creating/destroying an entity with ModelComponent to trigger output

## Reporting Issues
If you encounter any problems:
1. Note which feature is not working
2. Provide steps to reproduce
3. Check console output for error messages
4. Include model file details if relevant (format, size, mesh count)

## Success Criteria
The implementation is successful if:
✅ All keyboard shortcuts toggle appropriate windows
✅ White background improves model visibility
✅ Focus Entity positions camera to view selected entity
✅ Model Editor displays comprehensive model information
✅ Entity Editor shows detailed ModelComponent data
✅ Debug output helps diagnose rendering issues
✅ No performance degradation from new features
