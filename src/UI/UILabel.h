#pragma once
#include "UIWidget.h"

class Font;

// UILabel: A widget for displaying text using a Font
class UILabel : public UIWidget {
protected:
    std::string text;
    Font* font = nullptr;
    glm::vec4 textColor{1.0f, 1.0f, 1.0f, 1.0f};
    float textScale = 1.0f;
    
public:
    UILabel();
    virtual ~UILabel() = default;
    
    // Set label text
    void setText(const std::string& t) { text = t; }
    const std::string& getText() const { return text; }
    
    // Set font (should be loaded via AssetManager)
    void setFont(Font* f) { font = f; }
    Font* getFont() const { return font; }
    
    // Set text color
    void setTextColor(const glm::vec4& color) { textColor = color; }
    const glm::vec4& getTextColor() const { return textColor; }
    
    // Set text scale
    void setTextScale(float scale) { textScale = scale; }
    float getTextScale() const { return textScale; }
    
    // Override to generate text draw command
    virtual void GenerateDrawCommands(std::vector<UIDrawCommand>& commands) override;
};
