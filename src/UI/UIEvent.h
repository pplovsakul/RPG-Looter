#pragma once
#include "../vendor/glm/glm.hpp"

enum class UIEventType {
    MouseMove,
    MouseButtonPress,
    MouseButtonRelease,
    MouseScroll,
    KeyPress,
    KeyRelease
};

enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2
};

struct UIEvent {
    UIEventType type;
    glm::vec2 mousePosition{0.0f, 0.0f};
    MouseButton mouseButton{MouseButton::Left};
    int keyCode = -1;
    glm::vec2 scrollOffset{0.0f, 0.0f};
    bool handled = false;

    // Factory methods for convenience
    static UIEvent MouseMove(const glm::vec2& pos) {
        UIEvent e;
        e.type = UIEventType::MouseMove;
        e.mousePosition = pos;
        return e;
    }

    static UIEvent MouseButtonPress(const glm::vec2& pos, MouseButton button) {
        UIEvent e;
        e.type = UIEventType::MouseButtonPress;
        e.mousePosition = pos;
        e.mouseButton = button;
        return e;
    }

    static UIEvent MouseButtonRelease(const glm::vec2& pos, MouseButton button) {
        UIEvent e;
        e.type = UIEventType::MouseButtonRelease;
        e.mousePosition = pos;
        e.mouseButton = button;
        return e;
    }

    static UIEvent MouseScroll(const glm::vec2& pos, const glm::vec2& offset) {
        UIEvent e;
        e.type = UIEventType::MouseScroll;
        e.mousePosition = pos;
        e.scrollOffset = offset;
        return e;
    }

    static UIEvent KeyPress(int keyCode) {
        UIEvent e;
        e.type = UIEventType::KeyPress;
        e.keyCode = keyCode;
        return e;
    }

    static UIEvent KeyRelease(int keyCode) {
        UIEvent e;
        e.type = UIEventType::KeyRelease;
        e.keyCode = keyCode;
        return e;
    }
};
