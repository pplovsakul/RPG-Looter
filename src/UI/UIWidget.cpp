#include "UIWidget.h"
#include <algorithm>

UIWidget::UIWidget() {
}

void UIWidget::addChild(std::unique_ptr<UIWidget> child) {
    if (child) {
        child->parent = this;
        children.push_back(std::move(child));
    }
}

void UIWidget::removeChild(UIWidget* child) {
    if (!child) return;
    
    auto it = std::find_if(children.begin(), children.end(),
        [child](const std::unique_ptr<UIWidget>& ptr) {
            return ptr.get() == child;
        });
    
    if (it != children.end()) {
        (*it)->parent = nullptr;
        children.erase(it);
    }
}

void UIWidget::OnUpdate(float deltaTime) {
    // Update all children
    for (auto& child : children) {
        if (child && child->isVisible()) {
            child->OnUpdate(deltaTime);
        }
    }
}

bool UIWidget::OnEvent(UIEvent& event) {
    if (!enabled || !visible) {
        return false;
    }

    // Process children first (top to bottom in hierarchy)
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        if ((*it)->OnEvent(event)) {
            return true; // Event was handled by a child
        }
    }

    // If not handled by children, check if this widget should handle it
    return false;
}

void UIWidget::GenerateDrawCommands(std::vector<UIDrawCommand>& commands) {
    if (!visible) return;

    // Generate draw commands for children
    for (auto& child : children) {
        if (child && child->isVisible()) {
            child->GenerateDrawCommands(commands);
        }
    }
}

bool UIWidget::hitTest(const glm::vec2& point) const {
    if (!visible || !enabled) {
        return false;
    }
    
    glm::vec2 absPos = getAbsolutePosition();
    UIRect absRect(absPos.x, absPos.y, rect.width, rect.height);
    return absRect.contains(point);
}

glm::vec2 UIWidget::getAbsolutePosition() const {
    glm::vec2 pos(rect.x, rect.y);
    
    if (parent) {
        glm::vec2 parentPos = parent->getAbsolutePosition();
        pos.x += parentPos.x;
        pos.y += parentPos.y;
    }
    
    return pos;
}
