#include "UILabel.h"
#include "../Font.h"

UILabel::UILabel() {
    // Default constructor
}

void UILabel::GenerateDrawCommands(std::vector<UIDrawCommand>& commands) {
    if (!visible || text.empty() || !font) {
        return;
    }
    
    // Generate draw command for the text
    // Use absolute position for proper rendering in hierarchy
    glm::vec2 absPos = getAbsolutePosition();
    UIDrawCommand cmd;
    cmd.type = UIDrawCommand::Type::Text;
    cmd.rect = UIRect(absPos.x, absPos.y, rect.width, rect.height);
    cmd.text = text;
    cmd.font = font;
    cmd.color = textColor;
    cmd.textScale = textScale;
    commands.push_back(cmd);
    
    // Generate draw commands for children
    for (const auto& child : children) {
        if (child && child->isVisible()) {
            child->GenerateDrawCommands(commands);
        }
    }
}
