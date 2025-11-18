#include "UIPanel.h"

UIPanel::UIPanel() {
    // Default constructor
}

void UIPanel::GenerateDrawCommands(std::vector<UIDrawCommand>& commands) {
    if (!visible) {
        return;
    }
    
    // Generate draw command for the panel background
    // Use absolute position for proper rendering in hierarchy
    glm::vec2 absPos = getAbsolutePosition();
    UIDrawCommand cmd;
    cmd.type = UIDrawCommand::Type::Rectangle;
    cmd.rect = UIRect(absPos.x, absPos.y, rect.width, rect.height);
    cmd.color = backgroundColor;
    commands.push_back(cmd);
    
    // Generate draw commands for children
    for (const auto& child : children) {
        if (child && child->isVisible()) {
            child->GenerateDrawCommands(commands);
        }
    }
}
