#include "UIPanel.h"

UIPanel::UIPanel() {
    // Default constructor
}

void UIPanel::GenerateDrawCommands(std::vector<UIDrawCommand>& commands) {
    if (!visible) {
        return;
    }
    
    // Generate draw command for the panel background
    UIDrawCommand cmd;
    cmd.type = UIDrawCommand::Type::Rectangle;
    cmd.rect = rect;
    cmd.color = backgroundColor;
    commands.push_back(cmd);
    
    // Generate draw commands for children
    for (const auto& child : children) {
        if (child && child->isVisible()) {
            child->GenerateDrawCommands(commands);
        }
    }
}
