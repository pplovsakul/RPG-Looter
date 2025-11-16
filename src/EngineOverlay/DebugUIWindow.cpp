#include "DebugUIWindow.h"
#include "Widgets.h"
#include <cstdio>
#include <glad/glad.h>

namespace EngineUI {

DebugUIWindow::DebugUIWindow() {
}

void DebugUIWindow::render(bool* p_open) {
    UIContext* ctx = GetContext();
    if (!ctx) return;
    
    Rect initialRect(900, 400, 350, 400);
    if (!ctx->beginWindow("Debug UI", p_open, &initialRect)) {
        return;
    }
    
    Text("Render State Diagnostics");
    Separator();
    
    // OpenGL state
    Text("OpenGL State:");
    
    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    char blendText[64];
    snprintf(blendText, sizeof(blendText), "Blend: %s", blendEnabled ? "ON" : "OFF");
    TextColored(blendEnabled ? Color::Green() : Color::Red(), blendText);
    
    GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
    char depthText[64];
    snprintf(depthText, sizeof(depthText), "Depth Test: %s", depthTest ? "ON" : "OFF");
    TextColored(depthTest ? Color::Yellow() : Color::Green(), depthText);
    
    GLboolean cullFace = glIsEnabled(GL_CULL_FACE);
    char cullText[64];
    snprintf(cullText, sizeof(cullText), "Cull Face: %s", cullFace ? "ON" : "OFF");
    TextColored(cullFace ? Color::Yellow() : Color::Green(), cullText);
    
    GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
    char scissorText[64];
    snprintf(scissorText, sizeof(scissorText), "Scissor Test: %s", scissor ? "ON" : "OFF");
    TextColored(scissor ? Color::Green() : Color::Red(), scissorText);
    
    Separator();
    
    // Viewport info
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    char viewportText[128];
    snprintf(viewportText, sizeof(viewportText), "Viewport: %d x %d (at %d, %d)", 
            viewport[2], viewport[3], viewport[0], viewport[1]);
    Text(viewportText);
    
    // Scissor rect
    GLint scissorBox[4];
    glGetIntegerv(GL_SCISSOR_BOX, scissorBox);
    char scissorBoxText[128];
    snprintf(scissorBoxText, sizeof(scissorBoxText), "Scissor: %d x %d (at %d, %d)", 
            scissorBox[2], scissorBox[3], scissorBox[0], scissorBox[1]);
    Text(scissorBoxText);
    
    Separator();
    
    // Font atlas info
    Text("Font Atlas:");
    FontAtlas* fontAtlas = ctx->getFontAtlas();
    if (fontAtlas) {
        GLuint texID = fontAtlas->getTexture();
        char atlasText[128];
        snprintf(atlasText, sizeof(atlasText), "Texture ID: %u", texID);
        TextColored(texID > 0 ? Color::Green() : Color::Red(), atlasText);
        
        char lineHeightText[64];
        snprintf(lineHeightText, sizeof(lineHeightText), "Line Height: %.1f", fontAtlas->getLineHeight());
        Text(lineHeightText);
        
        // Try to measure some text
        glm::vec2 testSize = fontAtlas->measureText("Test");
        char measureText[64];
        snprintf(measureText, sizeof(measureText), "\"Test\" measures: %.1f x %.1f", testSize.x, testSize.y);
        Text(measureText);
    } else {
        TextColored(Color::Red(), "Font Atlas: NULL");
    }
    
    Separator();
    
    // Input state
    Text("Input State:");
    const InputState& input = ctx->getInput();
    char mousePosText[64];
    snprintf(mousePosText, sizeof(mousePosText), "Mouse: (%.1f, %.1f)", input.mousePos.x, input.mousePos.y);
    Text(mousePosText);
    
    char mouseButtonText[64];
    snprintf(mouseButtonText, sizeof(mouseButtonText), "Mouse Buttons: L:%d M:%d R:%d", 
            input.mouseDown[0], input.mouseDown[1], input.mouseDown[2]);
    Text(mouseButtonText);
    
    Separator();
    
    // Style info
    Text("Style:");
    Style& style = ctx->getStyle();
    char fontSizeText[64];
    snprintf(fontSizeText, sizeof(fontSizeText), "Font Size: %.1f", style.fontSize);
    Text(fontSizeText);
    
    char windowBgText[128];
    snprintf(windowBgText, sizeof(windowBgText), "Window BG: (%.2f, %.2f, %.2f, %.2f)", 
            style.windowBg.r, style.windowBg.g, style.windowBg.b, style.windowBg.a);
    Text(windowBgText);
    
    Separator();
    
    // Small font atlas preview
    Text("Font Atlas Preview:");
    Text("(Sample characters should be visible below)");
    TextColored(Color::Yellow(), "ABCDEF 012");
    TextColored(Color::Green(), "abcdef 345");
    TextColored(Color(1.0f, 0.5f, 0.0f, 1.0f), "Test: !@#$%");
    
    ctx->endWindow();
}

} // namespace EngineUI
