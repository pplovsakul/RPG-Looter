#include "Menu.h"
#include <iostream> // only for debug (optional)

Button::Button(float x_, float y_, float w_, float h_)
    : x(x_), y(y_), width(w_), height(h_), m_indices({ 0,1,2, 2,3,0 })
{
}

// center position (useful for RenderObject which expects centered geometry)
glm::vec2 Button::get_position() const {
    return { x + width * 0.5f, y + height * 0.5f };
}

glm::vec2 Button::get_size() const {
    return { width, height };
}

void Button::SetPosition(const glm::vec2& pos) {
    // pos is bottom-left corner to stay consistent with ctor
    x = pos.x;
    y = pos.y;
}

void Button::SetSize(const glm::vec2& size) {
    width = size.x;
    height = size.y;
}

bool Button::isHoveredInternal(double mouseX, double mouseY) const {
    return mouseX >= x && mouseX <= x + width &&
        mouseY >= y && mouseY <= y + height;
}

bool Button::IsHovered(GLFWwindow* window) const {
    double mouseX_d, mouseY_d;
    glfwGetCursorPos(window, &mouseX_d, &mouseY_d);

    int winW, winH;
    glfwGetWindowSize(window, &winW, &winH);
    // convert GLFW top-left y to bottom-left y
    double flippedY = static_cast<double>(winH) - mouseY_d;

    return isHoveredInternal(mouseX_d, flippedY);
}

bool Button::isClicked(GLFWwindow* window) {
    // get cursor
    double mouseX_d, mouseY_d;
    glfwGetCursorPos(window, &mouseX_d, &mouseY_d);

    int winW, winH;
    glfwGetWindowSize(window, &winW, &winH);
    double flippedY = static_cast<double>(winH) - mouseY_d;

    bool hoveredNow = isHoveredInternal(mouseX_d, flippedY);

    // hover callback on state change
    if (hoveredNow != prevHovered) {
        prevHovered = hoveredNow;
        if (onHover) onHover(hoveredNow);
    }

    int buttonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    bool pressedNow = (buttonState == GLFW_PRESS);

    bool clicked = false;
    // Edge detection: trigger on press-down while hovered (not on hold)
    if (hoveredNow && pressedNow && !prevMouseDown) {
        // falling/rising edge: pressed now but wasn't before
        if (onClick) onClick();
        clicked = true;
    }

    // update previous state for next frame
    prevMouseDown = pressedNow;

    return clicked;
}

std::vector<Vertex_Menu> Button::GetVertices() const {
    float halfW = width * 0.5f;
    float halfH = height * 0.5f;
    float z = 0.0f;

    // NOTE: vertices are centered around origin. Use get_position() for placement.
    std::vector<Vertex_Menu> vertices;
    vertices.reserve(4);

    vertices.push_back(Vertex_Menu{ glm::vec3(-halfW, -halfH, z), glm::vec2(0.0f, 0.0f) }); // bottom-left
    vertices.push_back(Vertex_Menu{ glm::vec3(halfW, -halfH, z), glm::vec2(1.0f, 0.0f) }); // bottom-right
    vertices.push_back(Vertex_Menu{ glm::vec3(halfW,  halfH, z), glm::vec2(1.0f, 1.0f) }); // top-right
    vertices.push_back(Vertex_Menu{ glm::vec3(-halfW,  halfH, z), glm::vec2(0.0f, 1.0f) }); // top-left

    return vertices;
}

std::vector<unsigned int> Button::GetIndices() const {
    return m_indices;
}

std::vector<glm::vec2> Button::GetTexCoords() const {
    return {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };
}
