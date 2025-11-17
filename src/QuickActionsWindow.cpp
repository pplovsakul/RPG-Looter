#include "QuickActionsWindow.h"
#include "ConsoleWindow.h"
#include "Components.h"

void QuickActionsWindow::update(EntityManager& em, float deltaTime) {
    auto& settings = GlobalSettings::getInstance();
    
    // Main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                ConsoleWindow::Info("New Scene - Not yet implemented");
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                ConsoleWindow::Info("Save Scene - Not yet implemented");
            }
            if (ImGui::MenuItem("Load Scene", "Ctrl+O")) {
                ConsoleWindow::Info("Load Scene - Not yet implemented");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "ESC")) {
                ConsoleWindow::Info("Exit application");
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                ConsoleWindow::Info("Undo - Not yet implemented");
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
                ConsoleWindow::Info("Redo - Not yet implemented");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                ConsoleWindow::Info("Copy - Not yet implemented");
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                ConsoleWindow::Info("Paste - Not yet implemented");
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Entity")) {
            if (ImGui::MenuItem("Create Empty", "Ctrl+Shift+N")) {
                Entity* e = em.createEntity();
                e->tag = "New Entity";
                ConsoleWindow::Info("Created empty entity: " + std::to_string(e->id));
            }
            if (ImGui::MenuItem("Create Sprite", "Ctrl+Shift+S")) {
                Entity* e = em.createEntity();
                e->tag = "Sprite";
                auto* t = e->addComponent<TransformComponent>();
                t->position = glm::vec2(960.0f, 540.0f);
                t->scale = glm::vec2(100.0f, 100.0f);
                auto* r = e->addComponent<RenderComponent>();
                r->meshName = "quad";
                r->shaderName = "default";
                ConsoleWindow::Info("Created sprite entity: " + std::to_string(e->id));
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete All Entities")) {
                auto entities = em.getAllEntities();
                int count = entities.size();
                for (auto* e : entities) {
                    em.destroyEntity(e->id);
                }
                ConsoleWindow::Warning("Deleted " + std::to_string(count) + " entities");
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Performance Monitor", "F1", settings.windowVisibility.showPerformanceWindow)) {
                settings.windowVisibility.showPerformanceWindow = !settings.windowVisibility.showPerformanceWindow;
            }
            if (ImGui::MenuItem("Console", "F2", settings.windowVisibility.showConsoleWindow)) {
                settings.windowVisibility.showConsoleWindow = !settings.windowVisibility.showConsoleWindow;
            }
            if (ImGui::MenuItem("Scene Hierarchy", "F3", settings.windowVisibility.showSceneHierarchy)) {
                settings.windowVisibility.showSceneHierarchy = !settings.windowVisibility.showSceneHierarchy;
            }
            if (ImGui::MenuItem("Entity Editor", "F4", settings.windowVisibility.showEntityEditor)) {
                settings.windowVisibility.showEntityEditor = !settings.windowVisibility.showEntityEditor;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Asset Manager", "F5", settings.windowVisibility.showAssetManager)) {
                settings.windowVisibility.showAssetManager = !settings.windowVisibility.showAssetManager;
            }
            if (ImGui::MenuItem("Model Editor", "F6", settings.windowVisibility.showModelEditor)) {
                settings.windowVisibility.showModelEditor = !settings.windowVisibility.showModelEditor;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Settings", nullptr, settings.windowVisibility.showSettingsWindow)) {
                settings.windowVisibility.showSettingsWindow = !settings.windowVisibility.showSettingsWindow;
            }
            if (ImGui::MenuItem("Quick Actions", nullptr, settings.windowVisibility.showQuickActions)) {
                settings.windowVisibility.showQuickActions = !settings.windowVisibility.showQuickActions;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Keyboard Shortcuts", "F12")) {
                showHelpWindow = !showHelpWindow;
            }
            if (ImGui::MenuItem("About")) {
                ConsoleWindow::Info("RPG-Looter - 2D Game Engine");
            }
            ImGui::EndMenu();
        }
        
        // Right-aligned stats
        ImGui::SameLine(ImGui::GetWindowWidth() - 250);
        ImGui::Text("FPS: %.1f | Entities: %zu", 
                   ImGui::GetIO().Framerate, 
                   em.getAllEntities().size());
        
        ImGui::EndMainMenuBar();
    }
    
    // Help window
    if (showHelpWindow) {
        drawHelpWindow();
    }
    
    // Only show Quick Actions window if visible in settings
    if (!settings.windowVisibility.showQuickActions) {
        return;
    }
    
    // Quick actions floating window (optional)
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 220, 30), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 0), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Quick Actions", &settings.windowVisibility.showQuickActions, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }
    drawQuickActions(em);
    ImGui::End();
}

void QuickActionsWindow::drawQuickActions(EntityManager& em) {
    ImGui::Text("Quick Tools");
    ImGui::Separator();
    
    if (ImGui::Button("Create Empty Entity", ImVec2(-1, 0))) {
        Entity* e = em.createEntity();
        e->tag = "QuickEntity";
        ConsoleWindow::Info("Created entity: " + std::to_string(e->id));
    }
    
    if (ImGui::Button("Create Sprite", ImVec2(-1, 0))) {
        Entity* e = em.createEntity();
        e->tag = "Sprite";
        auto* t = e->addComponent<TransformComponent>();
        t->position = glm::vec2(960.0f, 540.0f);
        t->scale = glm::vec2(100.0f, 100.0f);
        auto* r = e->addComponent<RenderComponent>();
        r->meshName = "quad";
        r->shaderName = "default";
        ConsoleWindow::Info("Created sprite: " + std::to_string(e->id));
    }
    
    ImGui::Separator();
    
    if (ImGui::Button("Clear Console", ImVec2(-1, 0))) {
        ConsoleWindow::Clear();
    }
    
    ImGui::Separator();
    ImGui::Text("Entity Count: %zu", em.getAllEntities().size());
}

void QuickActionsWindow::drawHelpWindow() {
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("Keyboard Shortcuts & Help", &showHelpWindow);
    
    if (ImGui::BeginTabBar("HelpTabs")) {
        if (ImGui::BeginTabItem("Shortcuts")) {
            drawKeyboardShortcuts();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Getting Started")) {
            ImGui::TextWrapped(
                "Welcome to RPG-Looter Engine!\n\n"
                "This is a 2D game engine built with OpenGL and ImGui. "
                "Use the editor windows to create entities, manage assets, and build your game.\n\n"
                "Key Features:\n"
                "- Entity Component System (ECS) architecture\n"
                "- Visual model editor for creating game objects\n"
                "- Asset management for textures, sounds, and models\n"
                "- Real-time performance monitoring\n"
                "- Console logging system\n\n"
                "To get started:\n"
                "1. Use the Entity Editor (F4) to create entities\n"
                "2. Add components like Transform and Render\n"
                "3. Use the Model Editor (F6) to create custom shapes\n"
                "4. Load assets through the Asset Manager (F5)\n"
            );
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("About")) {
            ImGui::Text("RPG-Looter 2D Game Engine");
            ImGui::Separator();
            ImGui::Text("Version: Development Build");
            ImGui::Text("Built with:");
            ImGui::BulletText("C++17");
            ImGui::BulletText("OpenGL 3.3");
            ImGui::BulletText("GLFW 3.4");
            ImGui::BulletText("ImGui");
            ImGui::BulletText("GLM");
            ImGui::BulletText("OpenAL");
            ImGui::Spacing();
            ImGui::Text("Features:");
            ImGui::BulletText("Entity Component System");
            ImGui::BulletText("2D Rendering");
            ImGui::BulletText("Audio System");
            ImGui::BulletText("Visual Editor Tools");
            ImGui::BulletText("Asset Management");
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
    
    ImGui::End();
}

void QuickActionsWindow::drawKeyboardShortcuts() {
    ImGui::Text("General Shortcuts");
    ImGui::Separator();
    
    ImGui::Columns(2);
    ImGui::Text("ESC"); ImGui::NextColumn();
    ImGui::Text("Exit Application"); ImGui::NextColumn();
    
    ImGui::Text("F12"); ImGui::NextColumn();
    ImGui::Text("Show/Hide Help"); ImGui::NextColumn();
    ImGui::Columns(1);
    
    ImGui::Spacing();
    ImGui::Text("File Operations");
    ImGui::Separator();
    
    ImGui::Columns(2);
    ImGui::Text("Ctrl+N"); ImGui::NextColumn();
    ImGui::Text("New Scene"); ImGui::NextColumn();
    
    ImGui::Text("Ctrl+S"); ImGui::NextColumn();
    ImGui::Text("Save Scene"); ImGui::NextColumn();
    
    ImGui::Text("Ctrl+O"); ImGui::NextColumn();
    ImGui::Text("Load Scene"); ImGui::NextColumn();
    ImGui::Columns(1);
    
    ImGui::Spacing();
    ImGui::Text("Entity Operations");
    ImGui::Separator();
    
    ImGui::Columns(2);
    ImGui::Text("Ctrl+Shift+N"); ImGui::NextColumn();
    ImGui::Text("Create Empty Entity"); ImGui::NextColumn();
    
    ImGui::Text("Ctrl+Shift+S"); ImGui::NextColumn();
    ImGui::Text("Create Sprite Entity"); ImGui::NextColumn();
    
    ImGui::Text("Del"); ImGui::NextColumn();
    ImGui::Text("Delete Selected Entity"); ImGui::NextColumn();
    ImGui::Columns(1);
    
    ImGui::Spacing();
    ImGui::Text("Window Toggles");
    ImGui::Separator();
    
    ImGui::Columns(2);
    ImGui::Text("F1"); ImGui::NextColumn();
    ImGui::Text("Performance Monitor"); ImGui::NextColumn();
    
    ImGui::Text("F2"); ImGui::NextColumn();
    ImGui::Text("Console"); ImGui::NextColumn();
    
    ImGui::Text("F3"); ImGui::NextColumn();
    ImGui::Text("Scene Hierarchy"); ImGui::NextColumn();
    
    ImGui::Text("F4"); ImGui::NextColumn();
    ImGui::Text("Entity Editor"); ImGui::NextColumn();
    
    ImGui::Text("F5"); ImGui::NextColumn();
    ImGui::Text("Asset Manager"); ImGui::NextColumn();
    
    ImGui::Text("F6"); ImGui::NextColumn();
    ImGui::Text("Model Editor"); ImGui::NextColumn();
    ImGui::Columns(1);
    
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
        "Note: Some shortcuts are placeholders and not yet functional.");
}
