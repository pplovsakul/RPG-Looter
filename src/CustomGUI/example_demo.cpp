#include "CustomGUI/CustomGUI.h"
#include "CustomGUI/GUIRenderer.h"
#include <iostream>

/*
 * CustomGUI Demo Example
 * 
 * This demonstrates the basic usage of the Custom Immediate-Mode GUI system.
 * To integrate this into your application:
 * 
 * 1. Include CustomGUI.h
 * 2. Create a context at startup
 * 3. Create a renderer and initialize it
 * 4. In your main loop:
 *    - Call newFrame() at the beginning
 *    - Create windows and widgets
 *    - Call endFrame() and render()
 * 5. Cleanup at shutdown
 */

int main() {
    std::cout << "CustomGUI Demo Example" << std::endl;
    std::cout << "======================" << std::endl;
    
    // Create GUI context
    CustomGUI::GUIContext* ctx = CustomGUI::createContext();
    std::cout << "✓ GUI Context created" << std::endl;
    
    // Create renderer
    CustomGUI::GUIRenderer renderer;
    if (renderer.init()) {
        std::cout << "✓ GUI Renderer initialized" << std::endl;
    } else {
        std::cerr << "✗ Failed to initialize GUI Renderer" << std::endl;
        return 1;
    }
    
    // Set display size (would come from your window in real app)
    CustomGUI::setDisplaySize(1920, 1080);
    
    // Example usage in a hypothetical main loop
    std::cout << "\nExample usage in main loop:\n" << std::endl;
    
    std::cout << R"(
    // In your main loop:
    while (running) {
        // Start new frame
        CustomGUI::newFrame();
        
        // Create main window
        if (CustomGUI::begin("Demo Window")) {
            // Text
            CustomGUI::text("Hello, Custom GUI System!");
            CustomGUI::textColored(CustomGUI::Color(1.0f, 0.5f, 0.0f, 1.0f), 
                                   "This is colored text");
            
            CustomGUI::separator();
            
            // Button
            static int clickCount = 0;
            if (CustomGUI::button("Click Me!")) {
                clickCount++;
                printf("Button clicked %d times!\n", clickCount);
            }
            
            // Checkbox
            static bool enableFeature = true;
            CustomGUI::checkbox("Enable Feature", &enableFeature);
            
            // Radio buttons
            static int selectedOption = 0;
            CustomGUI::radioButton("Option 1", &selectedOption, 0);
            CustomGUI::radioButton("Option 2", &selectedOption, 1);
            CustomGUI::radioButton("Option 3", &selectedOption, 2);
            
            CustomGUI::separator();
            
            // Slider
            static float sliderValue = 0.5f;
            if (CustomGUI::sliderFloat("My Slider", &sliderValue, 0.0f, 1.0f)) {
                printf("Slider value: %.2f\n", sliderValue);
            }
            
            // Integer slider
            static int intValue = 50;
            CustomGUI::sliderInt("Int Slider", &intValue, 0, 100);
            
            CustomGUI::separator();
            
            // Color editor
            static float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
            CustomGUI::colorEdit4("Color", color);
            
            CustomGUI::spacing();
            
            // Layout examples
            CustomGUI::text("Layout:");
            CustomGUI::text("Item 1");
            CustomGUI::sameLine();
            CustomGUI::text("Item 2");
            CustomGUI::sameLine();
            CustomGUI::text("Item 3");
            
            CustomGUI::indent();
            CustomGUI::text("Indented text");
            CustomGUI::unindent();
        }
        CustomGUI::end();
        
        // Settings window
        static bool showSettings = true;
        if (CustomGUI::begin("Settings", &showSettings)) {
            CustomGUI::text("Application Settings");
            
            // Theme selection
            if (CustomGUI::button("Dark Theme")) {
                CustomGUI::styleColorsDark();
            }
            CustomGUI::sameLine();
            if (CustomGUI::button("Light Theme")) {
                CustomGUI::styleColorsLight();
            }
            CustomGUI::sameLine();
            if (CustomGUI::button("Classic Theme")) {
                CustomGUI::styleColorsClassic();
            }
            
            CustomGUI::separator();
            
            // Style customization
            CustomGUI::GUIStyle& style = CustomGUI::getStyle();
            CustomGUI::sliderFloat("Rounding", &style.FrameRounding, 0.0f, 12.0f);
            CustomGUI::sliderFloat("Alpha", &style.Alpha, 0.0f, 1.0f);
        }
        CustomGUI::end();
        
        // End frame and render
        CustomGUI::endFrame();
        CustomGUI::render();
        
        // Render to OpenGL
        renderer.setDisplaySize(windowWidth, windowHeight);
        renderer.render(ctx->getDrawList());
        
        // Swap buffers, etc.
    }
    )" << std::endl;
    
    std::cout << "\n✓ Demo example completed successfully!" << std::endl;
    std::cout << "\nFeatures demonstrated:" << std::endl;
    std::cout << "  • Window management (Begin/End)" << std::endl;
    std::cout << "  • Text rendering (plain and colored)" << std::endl;
    std::cout << "  • Buttons" << std::endl;
    std::cout << "  • Checkboxes and radio buttons" << std::endl;
    std::cout << "  • Sliders (float and int)" << std::endl;
    std::cout << "  • Color editors" << std::endl;
    std::cout << "  • Layout controls (separator, spacing, indent, sameLine)" << std::endl;
    std::cout << "  • Theme switching" << std::endl;
    std::cout << "  • Style customization" << std::endl;
    
    // Cleanup
    renderer.shutdown();
    CustomGUI::destroyContext(ctx);
    
    std::cout << "\n✓ Cleanup completed" << std::endl;
    
    return 0;
}
