#pragma once
#include "UIWidget.h"

// UIPanel: A simple container that renders a colored rectangle
class UIPanel : public UIWidget {
protected:
    glm::vec4 backgroundColor{0.2f, 0.2f, 0.2f, 1.0f};
    
public:
    UIPanel();
    virtual ~UIPanel() = default;
    
    // Set panel background color
    void setBackgroundColor(const glm::vec4& color) { backgroundColor = color; }
    const glm::vec4& getBackgroundColor() const { return backgroundColor; }
    
    // Override to generate rectangle draw command
    virtual void GenerateDrawCommands(std::vector<UIDrawCommand>& commands) override;
};
