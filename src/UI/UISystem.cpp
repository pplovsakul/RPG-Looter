#include "UISystem.h"
#include "UIRenderer.h"
#include <iostream>

UISystem::UISystem() {
    // Create a default root widget
    rootWidget = std::make_unique<UIWidget>();
    rootWidget->setId("root");
    std::cout << "[UISystem] Initialized" << std::endl;
}

UISystem::~UISystem() {
    std::cout << "[UISystem] Destroyed" << std::endl;
}

void UISystem::update(EntityManager& em, float deltaTime) {
    // Update the UI tree
    if (rootWidget) {
        rootWidget->OnUpdate(deltaTime);
    }

    // Compute layout (for now, this is a placeholder)
    computeLayout();

    // Generate draw commands
    generateDrawCommands();

    // If we have a renderer, tell it to render
    if (renderer) {
        renderer->render(drawCommands);
    }
}

void UISystem::setRootWidget(std::unique_ptr<UIWidget> root) {
    rootWidget = std::move(root);
    if (rootWidget) {
        rootWidget->setId("root");
    }
}

bool UISystem::processEvent(const UIEvent& event) {
    if (!rootWidget) {
        return false;
    }

    // Update mouse position for tracking
    if (event.type == UIEventType::MouseMove || 
        event.type == UIEventType::MouseButtonPress || 
        event.type == UIEventType::MouseButtonRelease) {
        lastMousePosition = event.mousePosition;
    }

    // Create a mutable copy of the event
    UIEvent mutableEvent = event;
    
    // Process the event through the widget tree
    bool handled = rootWidget->OnEvent(mutableEvent);

    // Update mouseOverUI based on hit testing
    mouseOverUI = rootWidget->hitTest(lastMousePosition);

    return handled || mutableEvent.handled;
}

void UISystem::computeLayout() {
    // Placeholder for layout computation
    // In a full implementation, this would calculate positions and sizes
    // based on layout constraints, anchors, etc.
}

void UISystem::generateDrawCommands() {
    drawCommands.clear();
    
    if (rootWidget) {
        rootWidget->GenerateDrawCommands(drawCommands);
    }
}
