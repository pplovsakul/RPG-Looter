#include "UIButton.h"

UIButton::UIButton() {
    backgroundColor = normalColor;
}

bool UIButton::OnEvent(UIEvent& event) {
    if (!visible || !enabled) {
        return false;
    }
    
    if (event.type == UIEventType::MouseMove) {
        bool wasHovered = isHovered;
        isHovered = rect.contains(event.mousePosition);
        
        // Trigger hover/leave callbacks
        if (isHovered && !wasHovered && onHoverCallback) {
            onHoverCallback();
        } else if (!isHovered && wasHovered && onLeaveCallback) {
            onLeaveCallback();
        }
        
        updateBackgroundColor();
        return isHovered;
    }
    else if (event.type == UIEventType::MouseButtonPress) {
        if (event.mouseButton == MouseButton::Left) {
            if (rect.contains(event.mousePosition)) {
                isPressed = true;
                updateBackgroundColor();
                return true;
            }
        }
    }
    else if (event.type == UIEventType::MouseButtonRelease) {
        if (event.mouseButton == MouseButton::Left) {
            if (isPressed) {
                if (rect.contains(event.mousePosition)) {
                    // Button was clicked
                    if (onClickCallback) {
                        onClickCallback();
                    }
                }
                isPressed = false;
                updateBackgroundColor();
                return true;
            }
        }
    }
    
    return false;
}

void UIButton::OnUpdate(float deltaTime) {
    // Base class update
    UIPanel::OnUpdate(deltaTime);
}

void UIButton::updateBackgroundColor() {
    if (isPressed) {
        backgroundColor = pressedColor;
    } else if (isHovered) {
        backgroundColor = hoverColor;
    } else {
        backgroundColor = normalColor;
    }
}
