#include "ModelEditorWindow.h"
#include "AssetManager.h"
#include "Components.h"
#include <algorithm>
#include <cctype>

void ModelEditorWindow::update(EntityManager& em, float deltaTime) {
    auto& settings = GlobalSettings::getInstance();
    
    // Only show if the window is visible
    if (!settings.windowVisibility.showModelEditor) {
        return;
    }
    
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Model Editor", &settings.windowVisibility.showModelEditor)) {
        ImGui::End();
        return;
    }
    
    ImGui::Text("3D Model Inspector and Editor");
    ImGui::Separator();
    
    // Left panel: Model list
    ImGui::BeginChild("ModelListPanel", ImVec2(250, 0), true);
    drawModelList();
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // Right panel: Model details
    ImGui::BeginChild("ModelDetailsPanel", ImVec2(0, 0), true);
    drawModelDetails();
    ImGui::EndChild();
    
    ImGui::End();
}

void ModelEditorWindow::drawModelList() {
    ImGui::Text("Loaded Models");
    ImGui::Separator();
    
    // Search bar
    ImGui::InputTextWithHint("##modelSearch", "Search...", modelSearchBuffer, sizeof(modelSearchBuffer));
    ImGui::Separator();
    
    auto modelNames = AssetManager::getInstance()->getModelNames();
    
    // Filter by search
    std::vector<std::string> filteredModels;
    for (const auto& name : modelNames) {
        if (modelSearchBuffer[0] != '\0') {
            std::string nameLower = name;
            std::string searchLower = modelSearchBuffer;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
            if (nameLower.find(searchLower) == std::string::npos) continue;
        }
        filteredModels.push_back(name);
    }
    
    ImGui::Text("Models: %zu", filteredModels.size());
    ImGui::Spacing();
    
    // Model list
    for (size_t i = 0; i < filteredModels.size(); ++i) {
        const auto& name = filteredModels[i];
        bool isSelected = (selectedModelIndex == (int)i);
        
        if (ImGui::Selectable(name.c_str(), isSelected)) {
            selectedModelIndex = (int)i;
        }
    }
}

void ModelEditorWindow::drawModelDetails() {
    auto modelNames = AssetManager::getInstance()->getModelNames();
    
    if (selectedModelIndex < 0 || selectedModelIndex >= (int)modelNames.size()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No model selected");
        ImGui::Separator();
        ImGui::TextWrapped(
            "Select a model from the list on the left to view its details.\n\n"
            "The Model Editor allows you to:\n"
            "- Inspect model geometry and materials\n"
            "- View mesh statistics (vertices, triangles)\n"
            "- Check textures and colors\n"
            "- Preview model structure"
        );
        return;
    }
    
    const std::string& modelName = modelNames[selectedModelIndex];
    auto* model = AssetManager::getInstance()->getModel(modelName);
    
    if (!model) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: Model data not found");
        return;
    }
    
    ImGui::Text("Model: %s", modelName.c_str());
    ImGui::Separator();
    
    // Overview statistics
    if (ImGui::CollapsingHeader("Overview", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        ImGui::Text("Number of Meshes: %zu", model->meshes.size());
        
        size_t totalVertices = 0;
        size_t totalIndices = 0;
        for (const auto& mesh : model->meshes) {
            totalVertices += mesh.vertices.size();
            totalIndices += mesh.indices.size();
        }
        
        ImGui::Text("Total Vertices: %zu", totalVertices);
        ImGui::Text("Total Triangles: %zu", totalIndices / 3);
        ImGui::Unindent();
    }
    
    // Mesh details
    if (ImGui::CollapsingHeader("Mesh Details", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (size_t i = 0; i < model->meshes.size(); ++i) {
            const auto& mesh = model->meshes[i];
            ImGui::PushID((int)i);
            
            if (ImGui::TreeNode("Mesh", "Mesh %zu", i)) {
                ImGui::Indent();
                
                // Geometry info
                ImGui::Text("Vertices: %zu", mesh.vertices.size());
                ImGui::Text("Normals: %zu", mesh.normals.size());
                ImGui::Text("UV Coords: %zu", mesh.uvs.size());
                ImGui::Text("Indices: %zu", mesh.indices.size());
                ImGui::Text("Triangles: %zu", mesh.indices.size() / 3);
                
                ImGui::Spacing();
                ImGui::Separator();
                
                // Material info
                if (!mesh.materialName.empty()) {
                    ImGui::Text("Material: %s", mesh.materialName.c_str());
                }
                
                if (!mesh.textureName.empty()) {
                    ImGui::Text("Texture: %s", mesh.textureName.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No texture");
                }
                
                ImGui::Text("Color: (%.2f, %.2f, %.2f)", 
                           mesh.color.x, mesh.color.y, mesh.color.z);
                ImGui::ColorButton("Color Preview", 
                                  ImVec4(mesh.color.x, mesh.color.y, mesh.color.z, 1.0f),
                                  0, ImVec2(100, 30));
                
                ImGui::Spacing();
                ImGui::Separator();
                
                // Bounding box calculation
                if (!mesh.vertices.empty()) {
                    glm::vec3 minBounds = mesh.vertices[0];
                    glm::vec3 maxBounds = mesh.vertices[0];
                    
                    for (const auto& v : mesh.vertices) {
                        minBounds = glm::min(minBounds, v);
                        maxBounds = glm::max(maxBounds, v);
                    }
                    
                    glm::vec3 size = maxBounds - minBounds;
                    glm::vec3 center = (minBounds + maxBounds) * 0.5f;
                    
                    ImGui::Text("Bounding Box:");
                    ImGui::Indent();
                    ImGui::Text("Min: (%.2f, %.2f, %.2f)", minBounds.x, minBounds.y, minBounds.z);
                    ImGui::Text("Max: (%.2f, %.2f, %.2f)", maxBounds.x, maxBounds.y, maxBounds.z);
                    ImGui::Text("Size: (%.2f, %.2f, %.2f)", size.x, size.y, size.z);
                    ImGui::Text("Center: (%.2f, %.2f, %.2f)", center.x, center.y, center.z);
                    ImGui::Unindent();
                }
                
                ImGui::Unindent();
                ImGui::TreePop();
            }
            
            ImGui::PopID();
        }
    }
    
    // Data quality checks
    if (ImGui::CollapsingHeader("Data Quality")) {
        ImGui::Indent();
        
        bool hasWarnings = false;
        for (size_t i = 0; i < model->meshes.size(); ++i) {
            const auto& mesh = model->meshes[i];
            
            if (mesh.vertices.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
                                  "Mesh %zu: No vertices!", i);
                hasWarnings = true;
            }
            
            if (mesh.indices.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
                                  "Mesh %zu: No indices!", i);
                hasWarnings = true;
            }
            
            if (mesh.normals.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), 
                                  "Mesh %zu: No normals (using defaults)", i);
                hasWarnings = true;
            }
            
            if (mesh.uvs.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), 
                                  "Mesh %zu: No UV coordinates", i);
                hasWarnings = true;
            }
            
            if (mesh.indices.size() % 3 != 0) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
                                  "Mesh %zu: Index count not divisible by 3!", i);
                hasWarnings = true;
            }
        }
        
        if (!hasWarnings) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 
                              "All meshes have valid data!");
        }
        
        ImGui::Unindent();
    }
}

void ModelEditorWindow::drawModelPreview() {
    // Placeholder for future 3D preview implementation
    ImGui::Text("3D Preview (Coming Soon)");
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
                      "Future feature: Interactive 3D model preview");
}
