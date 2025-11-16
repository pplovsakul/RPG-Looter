#pragma once

class Component {
public:
    virtual ~Component() = default;
    bool enabled = true;
};