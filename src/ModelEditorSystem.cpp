#include "ModelEditorSystem.h"
#include "vendor/imgui/imgui.h"
#include "AssetManager.h"
#include "Texture.h"
#include "ModelSerializer.h"
#include <cmath>
#include <algorithm>
#include <cstring>

static ImVec2 modelPreviewSize = ImVec2(420, 360); // slightly narrower preview because left pane is larger

static bool pointInTriangle(const ImVec2& p, const ImVec2& a, const ImVec2& b, const ImVec2& c) {
    // Barycentric technique
    float v0x = c.x - a.x; float v0y = c.y - a.y;
    float v1x = b.x - a.x; float v1y = b.y - a.y;
    float v2x = p.x - a.x; float v2y = p.y - a.y;

    float dot00 = v0x*v0x + v0y*v0y;
    float dot01 = v0x*v1x + v0y*v1y;
    float dot02 = v0x*v2x + v0y*v2y;
    float dot11 = v1x*v1x + v1y*v1y;
    float dot12 = v1x*v2x + v1y*v2y;

    float denom = dot00 * dot11 - dot01 * dot01;
    if (fabs(denom) < 1e-6f) return false;
    float invDenom = 1.0f / denom;
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

static bool pointInConvexPoly(const ImVec2& p, const ImVec2* pts, int n) {
    // Use cross-product sign test (works for convex polygons)
    if (n < 3) return false;
    float prev = 0.0f;
    for (int i = 0; i < n; ++i) {
        const ImVec2& a = pts[i];
        const ImVec2& b = pts[(i+1)%n];
        float cross = (b.x - a.x)*(p.y - a.y) - (b.y - a.y)*(p.x - a.x);
        if (i == 0) prev = cross;
        else {
            // allow points on edge
            if (prev * cross < -1e-6f) return false;
            if (fabs(cross) > 1e-6f) prev = cross;
        }
    }
    return true;
}

void ModelEditorSystem::update(EntityManager& em, float /*deltaTime*/) {
    ImGui::SetNextWindowSize(ImVec2(900, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Model Editor");
    
    // Toolbar at the top
    drawToolbar();
    ImGui::Separator();

    // left column: make it larger (was 320)
    if (ImGui::BeginChild("LeftPane", ImVec2(420, 0), true)) {
        ImGui::Text("Add Shape");
        const char* types[] = { "Rectangle", "Triangle", "Circle", "Textured Quad" };
        ImGui::Combo("Type", &newShapeType, types, IM_ARRAYSIZE(types));

        static char addTexBuf[128] = "";
        if (newShapeType == 3) { // TexturedQuad index
            ImGui::InputText("Texture##add", addTexBuf, sizeof(addTexBuf));
            if (ImGui::Button("Add Textured Quad")) {
                ModelComponent::Shape s;
                s.type = ModelComponent::ShapeType::TexturedQuad;
                s.textureName = std::string(addTexBuf);
                workingShapes.push_back(s);
            }
        } else {
            if (ImGui::Button("Add Shape")) {
                ModelComponent::Shape s;
                switch (newShapeType) {
                case 0: s.type = ModelComponent::ShapeType::Rectangle; break;
                case 1: s.type = ModelComponent::ShapeType::Triangle; break;
                case 2: s.type = ModelComponent::ShapeType::Circle; break;
                default: s.type = ModelComponent::ShapeType::Rectangle; break;
                }
                workingShapes.push_back(s);
            }
        }

        ImGui::Separator();
        drawShapeList();
        ImGui::Separator();

        // Properties are now shown in the left pane so they are always visible when a shape is selected
        if (selectedShapeIndex >= 0 && selectedShapeIndex < (int)workingShapes.size()) {
            ImGui::Text("Selected Shape Properties");
            drawShapeProperties(workingShapes[selectedShapeIndex]);

            // quick transform widgets
            ImGui::Separator();
            ImGui::Text("Quick Transform");
            if (ImGui::Button("Reset Transform")) {
                workingShapes[selectedShapeIndex].position = glm::vec2(0.0f);
                workingShapes[selectedShapeIndex].rotation = 0.0f;
                workingShapes[selectedShapeIndex].scale = glm::vec2(1.0f);
            }
            ImGui::SameLine();
            if (ImGui::Button("Center")) {
                workingShapes[selectedShapeIndex].position = glm::vec2(0.0f);
            }

            ImGui::Separator();
            // Reorder / duplicate selected shape
            if (ImGui::SmallButton("Move Up")) {
                if (selectedShapeIndex > 0) { std::swap(workingShapes[selectedShapeIndex], workingShapes[selectedShapeIndex-1]); selectedShapeIndex--; }
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Move Down")) {
                if (selectedShapeIndex+1 < (int)workingShapes.size()) { std::swap(workingShapes[selectedShapeIndex], workingShapes[selectedShapeIndex+1]); selectedShapeIndex++; }
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Duplicate")) {
                workingShapes.insert(workingShapes.begin() + selectedShapeIndex + 1, workingShapes[selectedShapeIndex]);
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Remove")) {
                workingShapes.erase(workingShapes.begin() + selectedShapeIndex);
                selectedShapeIndex = -1;
            }
        } else {
            ImGui::TextDisabled("Select a shape to edit its properties here.");
        }

        ImGui::Separator();

        // --- Model Save / Register ---
        static char modelNameBuf[128] = "myModel";
        static char modelSavePath[256] = "res/models/myModel.json";
        ImGui::InputText("Model Name", modelNameBuf, sizeof(modelNameBuf));
        ImGui::InputText("Save Path", modelSavePath, sizeof(modelSavePath));
        if (ImGui::Button("Save to File")) {
            ModelComponent tmp; tmp.shapes = workingShapes;
            bool ok = ModelSerializer::saveModelToFile(tmp, modelSavePath);
            ImGui::SameLine();
            ImGui::TextColored(ok ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1), ok ? "Saved" : "Save failed");
        }
        ImGui::SameLine();
        if (ImGui::Button("Register as Asset")) {
            ModelComponent tmp; tmp.shapes = workingShapes;
            bool ok = AssetManager::getInstance()->addModel(std::string(modelNameBuf), tmp);
            ImGui::SameLine();
            ImGui::TextColored(ok ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1), ok ? "Registered" : "Register failed");
        }

        if (ImGui::Button("Load Model From File")) {
            std::unique_ptr<ModelComponent> loaded = ModelSerializer::loadModelFromFile(modelSavePath);
            if (loaded) {
                workingShapes = loaded->shapes;
                selectedShapeIndex = -1;
            }
        }

        ImGui::Separator();

        // Models in AssetManager
        ImGui::Text("Model Assets:");
        auto modelNames = AssetManager::getInstance()->getModelNames();
        for (const auto& n : modelNames) {
            ImGui::Text("%s", n.c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton((std::string("Load##") + n).c_str())) {
                ModelComponent* m = AssetManager::getInstance()->getModel(n);
                if (m) {
                    workingShapes = m->shapes;
                    selectedShapeIndex = -1;
                }
            }
        }

        ImGui::EndChild();
    }

    ImGui::SameLine();

    // right column: preview + assign panel
    if (ImGui::BeginChild("RightPane", ImVec2(0, 0), true)) {
        // View controls
        ImGui::Checkbox("Show Grid", &showGrid);
        ImGui::SameLine();
        ImGui::Checkbox("Snap to Grid", &snapToGrid);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::DragFloat("Grid Size", &gridSize, 1.0f, 5.0f, 100.0f);
        
        ImGui::Text("Preview (click to select, drag to move)");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Zoom: %.1fx", zoomLevel);

        // Preview canvas
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = modelPreviewSize;
        ImGui::InvisibleButton("canvas", canvasSize);
        bool hover = ImGui::IsItemHovered();
        ImVec2 mousePos = ImGui::GetIO().MousePos;

        // Background
        dl->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(30,30,30,255));
        
        // center in canvas
        ImVec2 center = ImVec2(canvasPos.x + canvasSize.x*0.5f + panOffset.x, canvasPos.y + canvasSize.y*0.5f + panOffset.y);
        
        // Draw grid if enabled
        if (showGrid) {
            drawGrid(dl, canvasPos, canvasSize, center);
        }
        
        // Draw center crosshair
        dl->AddLine(ImVec2(center.x - 10, center.y), ImVec2(center.x + 10, center.y), IM_COL32(100, 100, 100, 255));
        dl->AddLine(ImVec2(center.x, center.y - 10), ImVec2(center.x, center.y + 10), IM_COL32(100, 100, 100, 255));

        // Build index order sorted by layer then insertion order
        std::vector<int> indices(workingShapes.size());
        for (int i = 0; i < (int)indices.size(); ++i) indices[i] = i;
        std::stable_sort(indices.begin(), indices.end(), [this](int a, int b) {
            return workingShapes[a].layer < workingShapes[b].layer;
        });

        // handle mouse selection/drag start
        if (hover && ImGui::IsMouseClicked(0)) {
            int found = hitTestShapeAt(center, mousePos);
            selectedShapeIndex = found;
            if (found >= 0) { isDragging = true; draggingShapeIndex = found; }
            else { isDragging = false; draggingShapeIndex = -1; }
        }

        // handle dragging movement
        if (isDragging && ImGui::IsMouseDown(0) && draggingShapeIndex >= 0) {
            ImVec2 delta = ImGui::GetIO().MouseDelta;
            moveShapeBy(draggingShapeIndex, delta.x, -delta.y);
        } else {
            if (!ImGui::IsMouseDown(0)) { isDragging = false; draggingShapeIndex = -1; }
        }

        // draw shapes in sorted order
        for (int idx = 0; idx < (int)indices.size(); ++idx) {
            int i = indices[idx];
            const auto& s = workingShapes[i];
            ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(s.color.r, s.color.g, s.color.b, 1.0f));

            // apply rotation: compute rotated quad corners
            ImVec2 pos = ImVec2(center.x + s.position.x, center.y - s.position.y);
            ImVec2 half = ImVec2(s.size.x*0.5f * s.scale.x, s.size.y*0.5f * s.scale.y);

            // corners before rotation
            ImVec2 corners[4] = {
                ImVec2(-half.x, -half.y),
                ImVec2( half.x, -half.y),
                ImVec2( half.x,  half.y),
                ImVec2(-half.x,  half.y)
            };
            float rad = s.rotation * 3.14159265f / 180.0f;
            float cs = cosf(rad), sn = sinf(rad);
            ImVec2 worldCorners[4];
            for (int c = 0; c < 4; ++c) {
                worldCorners[c].x = pos.x + (corners[c].x * cs - corners[c].y * sn);
                worldCorners[c].y = pos.y + (corners[c].x * sn + corners[c].y * cs);
            }

            Texture* t = nullptr;
            if (!s.textureName.empty()) t = AssetManager::getInstance()->getTexture(s.textureName);

            if (t) {
                // ImDrawList doesn't support rotated textured quads directly.
                // Use AddImageQuad if available; otherwise draw axis-aligned image as fallback.
#ifdef IMGUI_HAS_ADD_IMAGE_QUAD
                dl->AddImageQuad((void*)(intptr_t)t->GetRendererID(),
                    worldCorners[0], worldCorners[1], worldCorners[2], worldCorners[3], IM_COL32_WHITE);
#else
                // fallback: draw axis-aligned image bounding box if rotation != 0
                // compute AABB of rotated quad
                float minx = worldCorners[0].x, maxx = worldCorners[0].x, miny = worldCorners[0].y, maxy = worldCorners[0].y;
                for (int c = 1; c < 4; ++c) {
                    minx = std::min(minx, worldCorners[c].x); maxx = std::max(maxx, worldCorners[c].x);
                    miny = std::min(miny, worldCorners[c].y); maxy = std::max(maxy, worldCorners[c].y);
                }
                dl->AddImage((void*)(intptr_t)t->GetRendererID(), ImVec2(minx, miny), ImVec2(maxx, maxy));
#endif
            } else {
                if (s.type == ModelComponent::ShapeType::Rectangle) {
                    // draw rotated rectangle via AddConvexPolyFilled or lines
                    ImVec2 pts[4] = { worldCorners[0], worldCorners[1], worldCorners[2], worldCorners[3] };
                    if (s.filled) dl->AddConvexPolyFilled(pts, 4, col);
                    else dl->AddPolyline(pts, 4, col, true, 2.0f);
                } else if (s.type == ModelComponent::ShapeType::Triangle) {
                    // build triangle from rotated rect (top and two bottom corners)
                    ImVec2 p1 = worldCorners[0]; // top-left after rotation may not be top; keep simple
                    ImVec2 p2 = worldCorners[1];
                    ImVec2 p3 = worldCorners[3];
                    if (s.filled) dl->AddTriangleFilled(p1, p2, p3, col);
                    else dl->AddTriangle(p1, p2, p3, col);
                } else if (s.type == ModelComponent::ShapeType::Circle) {
                    // circle rotation doesn't change appearance
                    if (s.filled) dl->AddCircleFilled(pos, half.x, col);
                    else dl->AddCircle(pos, half.x, col);
                } else if (s.type == ModelComponent::ShapeType::TexturedQuad) {
                    dl->AddConvexPolyFilled(worldCorners, 4, IM_COL32(100,100,100,255));
                }
            }

            if (i == selectedShapeIndex) {
                // draw selection outline (AABB of rotated quad)
                float minx = worldCorners[0].x, maxx = worldCorners[0].x, miny = worldCorners[0].y, maxy = worldCorners[0].y;
                for (int c = 1; c < 4; ++c) { minx = std::min(minx, worldCorners[c].x); maxx = std::max(maxx, worldCorners[c].x); miny = std::min(miny, worldCorners[c].y); maxy = std::max(maxy, worldCorners[c].y); }
                dl->AddRect(ImVec2(minx, miny), ImVec2(maxx, maxy), IM_COL32(255,255,0,255), 0.0f, 0, 2.0f);
            }
        }

        ImGui::Dummy(ImVec2(0, modelPreviewSize.y + 8));

        // --- Assign model asset to entity panel ---
        ImGui::Separator();
        ImGui::Text("Assign Model Asset to Entity");
        static int modelIdx = -1;
        auto models = AssetManager::getInstance()->getModelNames();
        if (ImGui::BeginCombo("Model##assign", modelIdx >= 0 && modelIdx < (int)models.size() ? models[modelIdx].c_str() : "None")) {
            for (int i = 0; i < (int)models.size(); ++i) {
                bool sel = (modelIdx == i);
                if (ImGui::Selectable(models[i].c_str(), sel)) modelIdx = i;
            }
            ImGui::EndCombo();
        }
        static int entIdx = -1;
        auto ents = em.getAllEntities();
        if (ImGui::BeginCombo("Entity##assign", entIdx >= 0 && entIdx < (int)ents.size() ? (std::to_string(ents[entIdx]->id) + " : " + ents[entIdx]->tag).c_str() : "None")) {
            for (int i = 0; i < (int)ents.size(); ++i) {
                std::string lbl = std::to_string(ents[i]->id) + " : " + ents[i]->tag;
                bool sel = (entIdx == i);
                if (ImGui::Selectable(lbl.c_str(), sel)) entIdx = i;
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("Assign Selected Model to Entity")) {
            if (modelIdx >= 0 && modelIdx < (int)models.size() && entIdx >= 0 && entIdx < (int)ents.size()) {
                ModelComponent* m = AssetManager::getInstance()->getModel(models[modelIdx]);
                Entity* e = ents[entIdx];
                if (m && e) {
                    if (e->hasComponent<ModelComponent>()) e->removeComponent<ModelComponent>();
                    auto* mc = e->addComponent<ModelComponent>();
                    mc->shapes = m->shapes;
                }
            }
        }

        ImGui::EndChild();
    }

    ImGui::End();
}

void ModelEditorSystem::drawShapeList() {
    ImGui::Text("Shapes:");
    for (int i = 0; i < (int)workingShapes.size(); ++i) {
        const auto& s = workingShapes[i];
        char label[256];
        const char* typeName = "Rect";
        switch (s.type) {
        case ModelComponent::ShapeType::Rectangle: typeName = "Rect"; break;
        case ModelComponent::ShapeType::Triangle: typeName = "Tri"; break;
        case ModelComponent::ShapeType::Circle: typeName = "Circ"; break;
        case ModelComponent::ShapeType::TexturedQuad: typeName = "Tex"; break;
        }
        snprintf(label, sizeof(label), "[%d] L:%d %s (%.0f x %.0f)", i, s.layer, typeName, s.size.x * s.scale.x, s.size.y * s.scale.y);
        if (ImGui::Selectable(label, selectedShapeIndex == i)) selectedShapeIndex = i;
    }
}

void ModelEditorSystem::drawShapeProperties(ModelComponent::Shape& s) {
    ImGui::Text("Properties");
    int type = (int)s.type;
    if (ImGui::Combo("Type", &type, "Rectangle\0Triangle\0Circle\0Textured Quad\0")) s.type = static_cast<ModelComponent::ShapeType>(type);
    ImGui::DragFloat2("Position", &s.position.x, 1.0f);
    ImGui::DragFloat("Rotation", &s.rotation, 1.0f);
    ImGui::DragFloat2("Size", &s.size.x, 1.0f);
    ImGui::DragFloat2("Scale", &s.scale.x, 0.01f, 0.01f, 10.0f);
    ImGui::ColorEdit3("Color", &s.color.x);
    ImGui::Checkbox("Filled", &s.filled);
    ImGui::InputInt("Layer", &s.layer);
    char buf[256] = {};
    strncpy(buf, s.textureName.c_str(), sizeof(buf)); buf[sizeof(buf)-1] = 0;
    if (ImGui::InputText("Texture", buf, sizeof(buf))) s.textureName = buf;
    if (!s.textureName.empty()) {
        Texture* t = AssetManager::getInstance()->getTexture(s.textureName);
        if (t) ImGui::Image((void*)(intptr_t)t->GetRendererID(), ImVec2(48,48));
        else ImGui::TextDisabled("Texture not loaded");
    }
}

int ModelEditorSystem::hitTestShapeAt(const ImVec2& canvasCenter, const ImVec2& mousePos) {
    // Return top-most shape index under mouse (considering rotation)
    for (int i = (int)workingShapes.size() - 1; i >= 0; --i) {
        const auto& s = workingShapes[i];
        ImVec2 pos = ImVec2(canvasCenter.x + s.position.x, canvasCenter.y - s.position.y);
        ImVec2 half = ImVec2(s.size.x*0.5f * s.scale.x, s.size.y*0.5f * s.scale.y);

        // corners before rotation
        ImVec2 corners[4] = {
            ImVec2(-half.x, -half.y),
            ImVec2( half.x, -half.y),
            ImVec2( half.x,  half.y),
            ImVec2(-half.x,  half.y)
        };
        float rad = s.rotation * 3.14159265f / 180.0f;
        float cs = cosf(rad), sn = sinf(rad);
        ImVec2 worldCorners[4];
        for (int c = 0; c < 4; ++c) {
            worldCorners[c].x = pos.x + (corners[c].x * cs - corners[c].y * sn);
            worldCorners[c].y = pos.y + (corners[c].x * sn + corners[c].y * cs);
        }

        if (s.type == ModelComponent::ShapeType::Circle) {
            float dx = mousePos.x - pos.x;
            float dy = mousePos.y - pos.y;
            float dist2 = dx*dx + dy*dy;
            float r = half.x;
            if (dist2 <= r*r) return i;
        } else if (s.type == ModelComponent::ShapeType::Triangle) {
            // triangle uses corners 0,1,3
            if (pointInTriangle(mousePos, worldCorners[0], worldCorners[1], worldCorners[3])) return i;
        } else {
            // rectangle / textured quad - test point in convex quad
            if (pointInConvexPoly(mousePos, worldCorners, 4)) return i;
        }
    }
    return -1;
}

void ModelEditorSystem::moveShapeBy(int idx, float dx, float dy) {
    if (idx < 0 || idx >= (int)workingShapes.size()) return;
    
    if (snapToGrid) {
        // Snap the new position to grid
        float newX = workingShapes[idx].position.x + dx;
        float newY = workingShapes[idx].position.y + dy;
        workingShapes[idx].position.x = snapToGridValue(newX);
        workingShapes[idx].position.y = snapToGridValue(newY);
    } else {
        workingShapes[idx].position.x += dx;
        workingShapes[idx].position.y += dy;
    }
}

float ModelEditorSystem::snapToGridValue(float value) {
    return std::round(value / gridSize) * gridSize;
}

void ModelEditorSystem::drawGrid(ImDrawList* dl, const ImVec2& canvasPos, const ImVec2& canvasSize, const ImVec2& center) {
    // Draw grid lines
    ImU32 gridColor = IM_COL32(60, 60, 60, 255);
    ImU32 gridColorStrong = IM_COL32(80, 80, 80, 255);
    
    // Vertical lines
    float startX = center.x - (int)(canvasSize.x * 0.5f / gridSize) * gridSize;
    float endX = center.x + (int)(canvasSize.x * 0.5f / gridSize) * gridSize;
    for (float x = startX; x <= endX; x += gridSize) {
        if (x < canvasPos.x || x > canvasPos.x + canvasSize.x) continue;
        
        // Stronger line at every 10th grid or at center
        bool isStrong = (std::abs(x - center.x) < 0.1f) || 
                       (std::abs(std::fmod(x - center.x, gridSize * 10.0f)) < 0.1f);
        ImU32 color = isStrong ? gridColorStrong : gridColor;
        
        dl->AddLine(ImVec2(x, canvasPos.y), ImVec2(x, canvasPos.y + canvasSize.y), color);
    }
    
    // Horizontal lines
    float startY = center.y - (int)(canvasSize.y * 0.5f / gridSize) * gridSize;
    float endY = center.y + (int)(canvasSize.y * 0.5f / gridSize) * gridSize;
    for (float y = startY; y <= endY; y += gridSize) {
        if (y < canvasPos.y || y > canvasPos.y + canvasSize.y) continue;
        
        bool isStrong = (std::abs(y - center.y) < 0.1f) || 
                       (std::abs(std::fmod(y - center.y, gridSize * 10.0f)) < 0.1f);
        ImU32 color = isStrong ? gridColorStrong : gridColor;
        
        dl->AddLine(ImVec2(canvasPos.x, y), ImVec2(canvasPos.x + canvasSize.x, y), color);
    }
}

void ModelEditorSystem::drawToolbar() {
    ImGui::Text("Model Editor Toolbar");
    ImGui::SameLine(200);
    
    if (ImGui::Button("New Model")) {
        workingShapes.clear();
        selectedShapeIndex = -1;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Clear All")) {
        ImGui::OpenPopup("ClearAllConfirm");
    }
    
    if (ImGui::BeginPopupModal("ClearAllConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Clear all shapes?");
        ImGui::Separator();
        if (ImGui::Button("Yes", ImVec2(120, 0))) {
            workingShapes.clear();
            selectedShapeIndex = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "| Shapes: %zu", workingShapes.size());
    
    ImGui::SameLine();
    if (selectedShapeIndex >= 0) {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "| Selected: %d", selectedShapeIndex);
    }
}
