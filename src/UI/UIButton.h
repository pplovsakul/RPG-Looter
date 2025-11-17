#pragma once
#include "UIPanel.h"
#include <functional>

// UIButton: An interactive button that inherits from UIPanel and responds to mouse events
class UIButton : public UIPanel {
protected:
    glm::vec4 normalColor{0.3f, 0.3f, 0.3f, 1.0f};
    glm::vec4 hoverColor{0.4f, 0.4f, 0.4f, 1.0f};
    glm::vec4 pressedColor{0.2f, 0.2f, 0.2f, 1.0f};
    
    bool isHovered = false;
    bool isPressed = false;
    
    // Callback functions
    std::function<void()> onClickCallback;
    std::function<void()> onHoverCallback;
    std::function<void()> onLeaveCallback;
    
public:
    UIButton();
    virtual ~UIButton() = default;
    
    // Set button colors for different states
    void setNormalColor(const glm::vec4& color) { normalColor = color; }
    void setHoverColor(const glm::vec4& color) { hoverColor = color; }
    void setPressedColor(const glm::vec4& color) { pressedColor = color; }
    
    // Set callbacks
    void setOnClick(std::function<void()> callback) { onClickCallback = callback; }
    void setOnHover(std::function<void()> callback) { onHoverCallback = callback; }
    void setOnLeave(std::function<void()> callback) { onLeaveCallback = callback; }
    
    // Override event handling
    virtual bool OnEvent(UIEvent& event) override;
    
    // Override update to manage button state
    virtual void OnUpdate(float deltaTime) override;
    
protected:
    void updateBackgroundColor();
};
