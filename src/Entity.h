#pragma once
#include "Component.h"
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <string>

class Entity {
private:
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components;

public:
    unsigned int id;
    bool active = true;
    std::string tag = "";

    Entity(unsigned int id);

    template<typename T>
    T* addComponent();

    template<typename T>
    T* getComponent();

    template<typename T>
    bool hasComponent();

    template<typename T>
    void removeComponent();
};

// Template implementations must be in header
template<typename T>
T* Entity::addComponent() {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    auto component = std::make_unique<T>();
    T* ptr = component.get();
    components[std::type_index(typeid(T))] = std::move(component);
    return ptr;
}

template<typename T>
T* Entity::getComponent() {
    auto it = components.find(std::type_index(typeid(T)));
    if (it != components.end()) {
        return static_cast<T*>(it->second.get());
    }
    return nullptr;
}

template<typename T>
bool Entity::hasComponent() {
    return components.find(std::type_index(typeid(T))) != components.end();
}

template<typename T>
void Entity::removeComponent() {
    components.erase(std::type_index(typeid(T)));
}