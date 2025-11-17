#pragma once
#include "System.h"
#include "Entitymanager.h"
#include "Components.h"
#include "vendor/imgui/imgui.h"
#include <string>
#include <vector>
#include <algorithm>

class EditorSystem : public System {
public:
    EditorSystem() = default;
    void update(EntityManager& em, float deltaTime) override;

private:
    int selectedEntityId = -1;
    char newEntityName[64] = "NewEntity";
    char searchBuffer[128] = "";
    
    // Component templates
    enum class ComponentTemplate {
        Empty,
        Sprite,
        AudioSource,
        CompleteEntity
    };
    
    // Clipboard for component copy/paste
    struct ComponentClipboard {
        bool hasTransform = false;
        bool hasRender = false;
        bool hasAudio = false;
        TransformComponent transform;
        RenderComponent render;
        AudioComponent audio;
    } clipboard;
    
    void drawEntityEditingTab(EntityManager& em);
    void drawSaveLoadTab(EntityManager& em);
    void drawTransformEditor(Entity* e);
    void drawRenderEditor(Entity* e);
    void drawAudioEditor(Entity* e);
    void drawUIEditor(Entity* e);
    void drawModelEditor(Entity* e);
    void drawComponentTemplates(EntityManager& em);
    void drawBatchOperations(EntityManager& em);
    void copyComponentsToClipboard(Entity* e);
    void pasteComponentsFromClipboard(Entity* e);
};