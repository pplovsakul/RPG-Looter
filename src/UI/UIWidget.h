#pragma once
#include "UIEvent.h"
#include "../vendor/glm/glm.hpp"
#include <vector>
#include <memory>
#include <string>

// Represents a rectangular area for layout
struct UIRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    UIRect() = default;
    UIRect(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}

    bool contains(const glm::vec2& point) const {
        return point.x >= x && point.x <= (x + width) &&
               point.y >= y && point.y <= (y + height);
    }
};

// Simple draw command for rendering
struct UIDrawCommand {
    enum class Type {
        Rectangle,
        Text
    };

    Type type;
    UIRect rect;
    glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    std::string text;
};

class UIWidget {
protected:
    UIWidget* parent = nullptr;
    std::vector<std::unique_ptr<UIWidget>> children;
    
    UIRect rect;
    bool visible = true;
    bool enabled = true;
    std::string id;

public:
    UIWidget();
    virtual ~UIWidget() = default;

    // Hierarchy management
    void addChild(std::unique_ptr<UIWidget> child);
    void removeChild(UIWidget* child);
    UIWidget* getParent() const { return parent; }
    const std::vector<std::unique_ptr<UIWidget>>& getChildren() const { return children; }

    // Layout properties
    void setRect(const UIRect& r) { rect = r; }
    const UIRect& getRect() const { return rect; }
    void setPosition(float x, float y) { rect.x = x; rect.y = y; }
    void setSize(float w, float h) { rect.width = w; rect.height = h; }

    // Visibility and interaction
    void setVisible(bool v) { visible = v; }
    bool isVisible() const { return visible; }
    void setEnabled(bool e) { enabled = e; }
    bool isEnabled() const { return enabled; }

    // Identifier
    void setId(const std::string& i) { id = i; }
    const std::string& getId() const { return id; }

    // Virtual methods for derived classes
    virtual void OnUpdate(float deltaTime);
    virtual bool OnEvent(UIEvent& event);
    virtual void GenerateDrawCommands(std::vector<UIDrawCommand>& commands);

    // Hit testing
    virtual bool hitTest(const glm::vec2& point) const;

protected:
    // Helper to get absolute position (accounting for parent)
    glm::vec2 getAbsolutePosition() const;
};
