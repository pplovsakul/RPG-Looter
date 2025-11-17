#pragma once
#include "System.h"
#include "Entitymanager.h"
#include "GlobalSettings.h"
#include "Components.h"
#include "AssetManager.h"
#include "vendor/imgui/imgui.h"
#include <vector>
#include <string>

class ModelEditorSystem : public System {
public:
    ModelEditorSystem() = default;
    void update(EntityManager& em, float deltaTime) override;

private:
    std::vector<ModelComponent::Shape> workingShapes;
    int newShapeType = 0; // index into ShapeType
    int selectedShapeIndex = -1;
    int targetEntityId = -1;

    // drag state for mouse interaction in preview
    bool isDragging = false;
    int draggingShapeIndex = -1;
    
    // Grid and snap settings
    bool showGrid = true;
    bool snapToGrid = false;
    float gridSize = 10.0f;
    
    // Zoom and pan
    float zoomLevel = 1.0f;
    ImVec2 panOffset = ImVec2(0, 0);

    // UI helpers
    void drawShapeList();
    void drawShapeProperties(ModelComponent::Shape& s);
    void drawPreview();
    void drawGrid(ImDrawList* dl, const ImVec2& canvasPos, const ImVec2& canvasSize, const ImVec2& center);
    void drawToolbar();

    // helpers
    // canvasCenter - center point (screen coords) of the model preview
    // mousePos - mouse position in screen coords
    int hitTestShapeAt(const ImVec2& canvasCenter, const ImVec2& mousePos);
    void moveShapeBy(int idx, float dx, float dy); // dx/dy in model units (pixels)
    float snapToGridValue(float value);
};