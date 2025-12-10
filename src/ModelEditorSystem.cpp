#include "ModelEditorSystem.h"
#include "vendor/imgui/imgui.h"
#include "ConsoleWindow.h"

// NOTE: ModelEditorSystem is deprecated. ModelComponent now stores 3D mesh data from OBJ files.
// For 3D model editing, use external tools and load via OBJLoader.
// This stub is kept to maintain compatibility with the system initialization.

void ModelEditorSystem::update(EntityManager& /*em*/, float /*deltaTime*/) {
    auto& settings = GlobalSettings::getInstance();
    
    // Only show if the window is visible
    if (!settings.windowVisibility.showModelEditor) {
        return;
    }
    
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Model Editor (Deprecated)", &settings.windowVisibility.showModelEditor)) {
        ImGui::End();
        return;
    }
    
    ImGui::TextWrapped("The Model Editor for 2D shapes is deprecated.");
    ImGui::Spacing();
    ImGui::TextWrapped("ModelComponent now stores 3D mesh data loaded from OBJ files.");
    ImGui::Spacing();
    ImGui::TextWrapped("To use 3D models:");
    ImGui::BulletText("Create models in external 3D software (Blender, Maya, etc.)");
    ImGui::BulletText("Export as .obj files");
    ImGui::BulletText("Load via AssetManager using OBJLoader");
    ImGui::Spacing();
    ImGui::TextWrapped("For 2D rendering, use RenderComponent with basic meshes (quad, circle, triangle).");
    
    ImGui::End();
}
