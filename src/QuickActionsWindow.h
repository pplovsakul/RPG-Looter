#pragma once
#include "System.h"
#include "Entitymanager.h"
#include "GlobalSettings.h"
#include "vendor/imgui/imgui.h"
#include <string>

class QuickActionsWindow : public System {
public:
    QuickActionsWindow() = default;
    void update(EntityManager& em, float deltaTime) override;
    
private:
    bool showHelpWindow = false;
    
    void drawQuickActions(EntityManager& em);
    void drawKeyboardShortcuts();
    void drawHelpWindow();
};
