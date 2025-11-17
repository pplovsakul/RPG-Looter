# UI Widget System Usage Example

This document demonstrates how to use the new UI widget system with font rendering.

## Loading a Font

```cpp
#include "AssetManager.h"

// Load the Public Sans font
AssetManager* assetMgr = AssetManager::getInstance();
Font* font = assetMgr->loadFont(
    "PublicSans",
    "res/fonts/public-sans.json",
    "res/fonts/public-sans.png"
);
```

## Creating UI Widgets

### UIPanel - Colored Rectangle Container

```cpp
#include "UI/UIPanel.h"

auto panel = std::make_unique<UIPanel>();
panel->setRect(UIRect(100, 100, 300, 200)); // x, y, width, height
panel->setBackgroundColor(glm::vec4(0.2f, 0.3f, 0.4f, 1.0f)); // RGBA
panel->setVisible(true);
```

### UILabel - Text Display

```cpp
#include "UI/UILabel.h"

auto label = std::make_unique<UILabel>();
label->setRect(UIRect(120, 120, 0, 0)); // Position (width/height not used for labels)
label->setText("Hello, World!");
label->setFont(font); // Use the loaded font
label->setTextColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // White text
label->setTextScale(1.0f);
label->setVisible(true);
```

### UIButton - Interactive Button

```cpp
#include "UI/UIButton.h"

auto button = std::make_unique<UIButton>();
button->setRect(UIRect(150, 250, 200, 50)); // x, y, width, height
button->setNormalColor(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
button->setHoverColor(glm::vec4(0.4f, 0.4f, 0.4f, 1.0f));
button->setPressedColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));

// Set callback for click event
button->setOnClick([]() {
    std::cout << "Button clicked!" << std::endl;
});

button->setOnHover([]() {
    std::cout << "Button hovered!" << std::endl;
});

button->setVisible(true);
button->setEnabled(true);
```

## Building a Widget Hierarchy

```cpp
#include "UI/UISystem.h"

// Get the UI system (assuming it's already set up in Game.cpp)
UISystem* uiSystem = /* obtain from game */;

// Create a panel as container
auto mainPanel = std::make_unique<UIPanel>();
mainPanel->setRect(UIRect(50, 50, 400, 300));
mainPanel->setBackgroundColor(glm::vec4(0.1f, 0.1f, 0.1f, 0.9f));

// Create a label
auto titleLabel = std::make_unique<UILabel>();
titleLabel->setRect(UIRect(70, 70, 0, 0));
titleLabel->setText("My UI Panel");
titleLabel->setFont(font);
titleLabel->setTextColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

// Create a button
auto okButton = std::make_unique<UIButton>();
okButton->setRect(UIRect(175, 280, 100, 40));
okButton->setOnClick([]() {
    std::cout << "OK clicked!" << std::endl;
});

// Add widgets as children
mainPanel->addChild(std::move(titleLabel));
mainPanel->addChild(std::move(okButton));

// Set as root widget
uiSystem->setRootWidget(std::move(mainPanel));
```

## Font System Details

### Font Metrics
The Font class provides access to:
- Glyph metrics (advance, bounds)
- Kerning information
- Atlas texture dimensions
- Line height and baseline

### SDF Rendering
The UIRenderer uses Signed Distance Field (SDF) rendering for crisp text at any scale:
- Sharp edges at various sizes
- Smooth antialiasing
- GPU-accelerated rendering
- Distance range of 4 pixels for good quality

## Build Integration

The font assets are automatically copied during build:
```cmake
# Fonts are copied from res/fonts/ to build/res/fonts/
# Accessible via FONT_DIR preprocessor definition
```

## Complete Example

```cpp
// In Game::setup() or similar initialization code
void setupUI() {
    // Load font
    AssetManager* assetMgr = AssetManager::getInstance();
    Font* font = assetMgr->loadFont(
        "PublicSans",
        FONT_DIR "/public-sans.json",
        FONT_DIR "/public-sans.png"
    );
    
    // Create UI hierarchy
    auto root = std::make_unique<UIPanel>();
    root->setRect(UIRect(0, 0, 800, 600));
    root->setBackgroundColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent
    
    // Add a welcome message
    auto welcomeLabel = std::make_unique<UILabel>();
    welcomeLabel->setRect(UIRect(100, 100, 0, 0));
    welcomeLabel->setText("Welcome to RPG-Looter!");
    welcomeLabel->setFont(font);
    welcomeLabel->setTextScale(1.5f);
    
    // Add a start button
    auto startButton = std::make_unique<UIButton>();
    startButton->setRect(UIRect(300, 250, 200, 60));
    startButton->setOnClick([]() {
        // Start game logic
        std::cout << "Game started!" << std::endl;
    });
    
    // Add button label
    auto buttonLabel = std::make_unique<UILabel>();
    buttonLabel->setRect(UIRect(350, 265, 0, 0));
    buttonLabel->setText("Start Game");
    buttonLabel->setFont(font);
    startButton->addChild(std::move(buttonLabel));
    
    root->addChild(std::move(welcomeLabel));
    root->addChild(std::move(startButton));
    
    // Set as UI root
    uiSystem->setRootWidget(std::move(root));
}
```

## Event Handling

UI events are processed through the UISystem:

```cpp
// In your input handling code
void handleInput(double mouseX, double mouseY, int button, int action) {
    glm::vec2 pos(mouseX, mouseY);
    
    if (action == GLFW_PRESS) {
        UIEvent event = UIEvent::MouseButtonPress(pos, MouseButton::Left);
        uiSystem->processEvent(event);
    }
    else if (action == GLFW_RELEASE) {
        UIEvent event = UIEvent::MouseButtonRelease(pos, MouseButton::Left);
        uiSystem->processEvent(event);
    }
}

void handleMouseMove(double mouseX, double mouseY) {
    glm::vec2 pos(mouseX, mouseY);
    UIEvent event = UIEvent::MouseMove(pos);
    uiSystem->processEvent(event);
}
```
