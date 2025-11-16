#pragma once
#include "System.h"
#include "Entitymanager.h"
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

    // UI helpers
    void drawShapeList();
    void drawShapeProperties(ModelComponent::Shape& s);
    void drawPreview();

    // helpers
    // canvasCenter - center point (screen coords) of the model preview
    // mousePos - mouse position in screen coords
    int hitTestShapeAt(const ImVec2& canvasCenter, const ImVec2& mousePos);
    void moveShapeBy(int idx, float dx, float dy); // dx/dy in model units (pixels)
};