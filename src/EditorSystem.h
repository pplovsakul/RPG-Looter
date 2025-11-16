#pragma once
#include "System.h"
#include "Entitymanager.h"
#include "Components.h"
#include "vendor/imgui/imgui.h"
#include <string>

class EditorSystem : public System {
public:
    EditorSystem() = default;
    void update(EntityManager& em, float deltaTime) override;

private:
    int selectedEntityId = -1;
    char newEntityName[64] = "NewEntity";
    void drawTransformEditor(Entity* e);
    void drawRenderEditor(Entity* e);
    void drawAudioEditor(Entity* e);
    void drawUIEditor(Entity* e);
    void drawModelEditor(Entity* e);
};