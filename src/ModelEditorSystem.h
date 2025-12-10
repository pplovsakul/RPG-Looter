#pragma once
#include "System.h"
#include "Entitymanager.h"
#include "GlobalSettings.h"
#include "Components.h"
#include "AssetManager.h"
#include "vendor/imgui/imgui.h"
#include <vector>
#include <string>

// NOTE: ModelEditorSystem is deprecated. ModelComponent now stores 3D mesh data from OBJ files.
// For 3D model editing, use external tools and load via OBJLoader.
// This stub is kept to maintain compatibility with the system initialization.
class ModelEditorSystem : public System {
public:
    ModelEditorSystem() = default;
    void update(EntityManager& em, float deltaTime) override;
};