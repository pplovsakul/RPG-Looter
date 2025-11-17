#pragma once
#include "../System.h"
#include "UIWidget.h"
#include "UIEvent.h"
#include <memory>
#include <vector>

// Forward declarations
class UIRenderer;
class InputSystem;

class UISystem : public System {
private:
    std::unique_ptr<UIWidget> rootWidget;
    std::vector<UIDrawCommand> drawCommands;
    UIRenderer* renderer = nullptr;
    
    // Cache for mouse position
    glm::vec2 lastMousePosition{0.0f, 0.0f};
    bool mouseOverUI = false;

public:
    UISystem();
    ~UISystem() override;

    // System interface
    void update(EntityManager& em, float deltaTime) override;

    // UI tree management
    UIWidget* getRootWidget() { return rootWidget.get(); }
    void setRootWidget(std::unique_ptr<UIWidget> root);

    // Event processing
    bool processEvent(const UIEvent& event);

    // Layout calculation (can be expanded)
    void computeLayout();

    // Draw command generation
    void generateDrawCommands();
    const std::vector<UIDrawCommand>& getDrawCommands() const { return drawCommands; }

    // Renderer management
    void setRenderer(UIRenderer* r) { renderer = r; }
    UIRenderer* getRenderer() const { return renderer; }

    // Mouse over UI tracking
    void setMouseOverUI(bool overUI) { mouseOverUI = overUI; }
    bool isMouseOverUI() const { return mouseOverUI; }
    
    // Update mouse position
    void updateMousePosition(const glm::vec2& pos) { lastMousePosition = pos; }
    const glm::vec2& getMousePosition() const { return lastMousePosition; }
};
