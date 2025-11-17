# Custom UI System - Developer Guide

## Overview

This project now includes a custom retained-mode UI system built on OpenGL. The system provides a foundation for creating interactive user interfaces without relying on immediate-mode libraries like ImGui for custom UI elements.

## Architecture

```
InputSystem → UISystem → UIRenderer
     ↓           ↓            ↓
Mouse/Keys → UI Tree → OpenGL Draw Calls
```

### Core Components

1. **UIWidget** - Base class for all UI elements
2. **UISystem** - Manages the UI tree and coordinates updates
3. **UIRenderer** - Renders UI using OpenGL
4. **UIEvent** - Event structures for interaction
5. **InputSystem** - Extended with mouse support

## Quick Start

### Creating a Simple UI Widget

```cpp
// Create a custom widget by inheriting from UIWidget
class MyButton : public UIWidget {
public:
    MyButton() {
        setSize(100.0f, 50.0f);
        setPosition(100.0f, 100.0f);
    }
    
    void OnUpdate(float deltaTime) override {
        UIWidget::OnUpdate(deltaTime); // Update children
        // Add custom update logic here
    }
    
    bool OnEvent(UIEvent& event) override {
        if (event.type == UIEventType::MouseButtonPress) {
            if (hitTest(event.mousePosition)) {
                std::cout << "Button clicked!" << std::endl;
                event.handled = true;
                return true;
            }
        }
        return UIWidget::OnEvent(event); // Let children handle
    }
    
    void GenerateDrawCommands(std::vector<UIDrawCommand>& commands) override {
        // Draw a colored rectangle
        UIDrawCommand cmd;
        cmd.type = UIDrawCommand::Type::Rectangle;
        cmd.rect = getRect();
        cmd.color = glm::vec4(0.3f, 0.6f, 1.0f, 1.0f); // Blue
        commands.push_back(cmd);
        
        UIWidget::GenerateDrawCommands(commands); // Draw children
    }
};
```

### Adding Widgets to the UI

```cpp
// In your game code (e.g., setupUI())
auto button = std::make_unique<MyButton>();
button->setId("myButton");
uiSystem->getRootWidget()->addChild(std::move(button));
```

### Processing UI Events

```cpp
// Create and send events to the UI system
UIEvent mouseEvent = UIEvent::MouseButtonPress(
    glm::vec2(mouseX, mouseY),
    MouseButton::Left
);

if (uiSystem->processEvent(mouseEvent)) {
    // Event was handled by UI, don't process in game
}
```

## API Reference

### UIWidget

#### Properties
```cpp
void setRect(const UIRect& r);
const UIRect& getRect() const;
void setPosition(float x, float y);
void setSize(float w, float h);
void setVisible(bool v);
bool isVisible() const;
void setEnabled(bool e);
bool isEnabled() const;
void setId(const std::string& id);
const std::string& getId() const;
```

#### Hierarchy
```cpp
void addChild(std::unique_ptr<UIWidget> child);
void removeChild(UIWidget* child);
UIWidget* getParent() const;
const std::vector<std::unique_ptr<UIWidget>>& getChildren() const;
```

#### Virtual Methods (Override in derived classes)
```cpp
virtual void OnUpdate(float deltaTime);
virtual bool OnEvent(UIEvent& event);
virtual void GenerateDrawCommands(std::vector<UIDrawCommand>& commands);
virtual bool hitTest(const glm::vec2& point) const;
```

### UISystem

```cpp
// Get root widget to add children
UIWidget* getRootWidget();

// Process events
bool processEvent(const UIEvent& event);

// Mouse tracking
void setMouseOverUI(bool overUI);
bool isMouseOverUI() const;
void updateMousePosition(const glm::vec2& pos);
const glm::vec2& getMousePosition() const;

// Renderer management
void setRenderer(UIRenderer* r);
UIRenderer* getRenderer() const;
```

### UIEvent

#### Factory Methods
```cpp
static UIEvent MouseMove(const glm::vec2& pos);
static UIEvent MouseButtonPress(const glm::vec2& pos, MouseButton button);
static UIEvent MouseButtonRelease(const glm::vec2& pos, MouseButton button);
static UIEvent MouseScroll(const glm::vec2& pos, const glm::vec2& offset);
static UIEvent KeyPress(int keyCode);
static UIEvent KeyRelease(int keyCode);
```

#### Event Types
- `UIEventType::MouseMove`
- `UIEventType::MouseButtonPress`
- `UIEventType::MouseButtonRelease`
- `UIEventType::MouseScroll`
- `UIEventType::KeyPress`
- `UIEventType::KeyRelease`

### InputSystem Extensions

```cpp
// Get current mouse position
glm::vec2 GetMousePosition() const;

// Check mouse button state
bool IsMouseButtonPressed(int button) const;

// UI input consumption
void SetMouseOverUI(bool overUI);
bool IsMouseOverUI() const;
```

### UIRenderer

```cpp
// Initialize (called automatically by Game)
void init();

// Render UI (called by UISystem)
void render(const std::vector<UIDrawCommand>& commands);

// Window size handling
void updateWindowSize(int width, int height);
void setProjectionMatrix(const glm::mat4& proj);
```

## Examples

### Example 1: Simple Panel

```cpp
class Panel : public UIWidget {
private:
    glm::vec4 backgroundColor{0.2f, 0.2f, 0.2f, 0.9f};
    
public:
    void setBackgroundColor(const glm::vec4& color) {
        backgroundColor = color;
    }
    
    void GenerateDrawCommands(std::vector<UIDrawCommand>& commands) override {
        UIDrawCommand cmd;
        cmd.type = UIDrawCommand::Type::Rectangle;
        cmd.rect = getRect();
        cmd.color = backgroundColor;
        commands.push_back(cmd);
        
        UIWidget::GenerateDrawCommands(commands); // Draw children
    }
};
```

### Example 2: Interactive Button

```cpp
class Button : public UIWidget {
private:
    std::function<void()> onClick;
    glm::vec4 normalColor{0.3f, 0.6f, 1.0f, 1.0f};
    glm::vec4 hoverColor{0.4f, 0.7f, 1.0f, 1.0f};
    glm::vec4 pressedColor{0.2f, 0.5f, 0.9f, 1.0f};
    bool isHovered = false;
    bool isPressed = false;
    
public:
    void setOnClick(std::function<void()> callback) {
        onClick = callback;
    }
    
    bool OnEvent(UIEvent& event) override {
        if (!isEnabled()) return false;
        
        bool wasHandled = UIWidget::OnEvent(event);
        if (wasHandled) return true;
        
        if (event.type == UIEventType::MouseMove) {
            isHovered = hitTest(event.mousePosition);
        }
        else if (event.type == UIEventType::MouseButtonPress) {
            if (hitTest(event.mousePosition)) {
                isPressed = true;
                event.handled = true;
                return true;
            }
        }
        else if (event.type == UIEventType::MouseButtonRelease) {
            if (isPressed && hitTest(event.mousePosition)) {
                if (onClick) onClick();
                event.handled = true;
            }
            isPressed = false;
        }
        
        return event.handled;
    }
    
    void GenerateDrawCommands(std::vector<UIDrawCommand>& commands) override {
        UIDrawCommand cmd;
        cmd.type = UIDrawCommand::Type::Rectangle;
        cmd.rect = getRect();
        
        if (isPressed)
            cmd.color = pressedColor;
        else if (isHovered)
            cmd.color = hoverColor;
        else
            cmd.color = normalColor;
        
        commands.push_back(cmd);
        UIWidget::GenerateDrawCommands(commands);
    }
};
```

### Example 3: Using the Button

```cpp
void setupUI() {
    auto button = std::make_unique<Button>();
    button->setPosition(400.0f, 300.0f);
    button->setSize(200.0f, 60.0f);
    button->setOnClick([]() {
        std::cout << "Button was clicked!" << std::endl;
    });
    
    uiSystem->getRootWidget()->addChild(std::move(button));
}
```

## Integration with Game

The UI system is automatically integrated into the game loop:

1. **InputSystem** updates first, collecting raw input
2. **UISystem** processes UI events and updates widgets
3. **UIRenderer** renders the UI (controlled by UISystem)
4. Other game systems process input (if not consumed by UI)

### Checking UI Input Consumption

```cpp
void handleGameInput() {
    // Don't process game input if UI has consumed it
    if (inputSystem->IsMouseOverUI()) {
        return; // UI is handling the input
    }
    
    // Process game input here
}
```

## Coordinate System

The UI uses a screen-space coordinate system:
- **Origin**: Top-left corner (0, 0)
- **X-axis**: Increases to the right
- **Y-axis**: Increases downward
- **Units**: Pixels

## Best Practices

1. **Always call base class methods** in overridden functions to ensure children are processed
2. **Mark events as handled** when you consume them (`event.handled = true`)
3. **Use hit testing** before processing mouse events (`hitTest(event.mousePosition)`)
4. **Set widget IDs** for debugging and widget lookup
5. **Manage ownership** carefully - widgets own their children via unique_ptr
6. **Update visibility** to hide widgets instead of removing them when possible
7. **Use absolute positioning** helper to get screen coordinates

## Troubleshooting

### UI not rendering
- Check that UIRenderer is initialized (`uiRenderer->init()`)
- Verify UISystem has a renderer set (`uiSystem->setRenderer(uiRenderer.get())`)
- Ensure widgets are visible (`widget->setVisible(true)`)
- Check that GenerateDrawCommands is adding commands

### Events not working
- Verify widget is enabled (`widget->setEnabled(true)`)
- Check event order (children process before parents)
- Ensure event.handled is being set when consumed
- Verify mouse position is in widget bounds

### Widgets not appearing at correct position
- Remember Y-axis points down from top-left
- Use getAbsolutePosition() to account for parent position
- Check window resize handling

## Future Enhancements

The current implementation provides a foundation. Future additions could include:

- **Text rendering** with font atlas
- **Texture support** for sprites and images
- **Layout managers** (flex, grid, anchors)
- **Standard widgets** (TextBox, Slider, Checkbox)
- **Animation system** for smooth transitions
- **Focus management** for keyboard navigation
- **Styling/theming** system
- **Drag and drop** support
- **Tooltip system**

## Performance Considerations

- Widget updates only occur for visible widgets
- Event processing stops when handled
- Draw commands are generated once per frame
- Geometry (quad) is reused for all rectangles
- Consider widget pooling for dynamic UIs with many elements

---

For questions or issues, refer to the source code in `src/UI/` or check the Git commit history for implementation details.
