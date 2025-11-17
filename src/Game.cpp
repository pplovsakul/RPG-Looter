#include "Game.h"

#include "InputSystem.h"
#include "RenderSystem.h"
#include "AudioSystem.h"
#include "EditorSystem.h"
#include "AssetManagerWindow.h"
#include "ModelEditorSystem.h"
#include "CollisionSystem.h"
#include "PerformanceWindow.h"
#include "ConsoleWindow.h"
#include "SceneHierarchyWindow.h"
#include "SettingsWindow.h"
#include "QuickActionsWindow.h"
#include "UI/UISystem.h"
#include "UI/UIRenderer.h"

#include "Components.h"

#include "vendor/glm/gtc/matrix_transform.hpp"

Game::~Game() = default;  // Defined here where UIRenderer is complete

void Game::update(float deltaTime) {
    auto& settings = GlobalSettings::getInstance();
    
    // Update auto-save timer
    if (settings.editorSettings.autoSaveEnabled) {
        settings.editorSettings.autoSaveTimer += deltaTime;
        if (settings.editorSettings.autoSaveTimer >= settings.editorSettings.autoSaveInterval) {
            settings.editorSettings.autoSaveTimer = 0.0f;
            ConsoleWindow::Info("Auto-save triggered (not yet fully implemented)");
        }
    }
    
    // Update systems based on their enabled state
    for (auto& system : systems) {
        // Check if system should be skipped based on control flags
        bool shouldUpdate = true;
        
        if (dynamic_cast<RenderSystem*>(system.get()) && !settings.systemControls.renderingEnabled) {
            shouldUpdate = false;
        }
        if (dynamic_cast<AudioSystem*>(system.get()) && !settings.systemControls.audioEnabled) {
            shouldUpdate = false;
        }
        if (dynamic_cast<CollisionSystem*>(system.get()) && !settings.systemControls.physicsEnabled) {
            shouldUpdate = false;
        }
        
        if (shouldUpdate) {
            system->update(entityManager, deltaTime);
        }
    }
    entityManager.cleanup();

    if (collisionSystem && settings.systemControls.physicsEnabled) {
        static int dbgCounter = 0;
        dbgCounter++;
        auto cols = collisionSystem->getCollisions();
        if (!cols.empty() || dbgCounter % 60 == 0) {
            std::cout << "[CollisionSystem] Collisions: " << cols.size();
            for (auto& p : cols) {
                if (p.first && p.second)
                    std::cout << " (" << p.first->id << "," << p.second->id << ")";
            }
            std::cout << "\n";
        }
    }
};

void Game::setAudioAvailable(bool available) {
    audioAvailable = available;
}

void Game::setup(GLFWwindow* window) {
    gameWindow = window; // Store window reference
    setupSystems(window);
    setupEntities();
}

void Game::applyVSync() {
    auto& settings = GlobalSettings::getInstance();
    if (gameWindow) {
        glfwSwapInterval(settings.renderingSettings.vsyncEnabled ? 1 : 0);
    }
}

void Game::setupSystems(GLFWwindow* window) {
    // Input System
    systems.push_back(std::make_unique<InputSystem>(window));

    // UI System - should be updated after InputSystem but before game actions
    auto uiSys = std::make_unique<UISystem>();
    uiSystem = uiSys.get();
    systems.push_back(std::move(uiSys));

    // UI Renderer
    uiRenderer = std::make_unique<UIRenderer>();
    uiRenderer->init();
    
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    uiRenderer->updateWindowSize(width, height);
    
    // Connect UIRenderer to UISystem
    if (uiSystem) {
        uiSystem->setRenderer(uiRenderer.get());
    }

    // Render System
    auto renderSys = std::make_unique<RenderSystem>();
    renderSys->init();
    renderSys->setViewMatrix(glm::mat4(1.0f));

    renderSys->setProjectionMatrix(glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f));
    renderSystem = renderSys.get(); // Cache pointer
    systems.push_back(std::move(renderSys));

    // Optional Audio System
    if (audioAvailable) {
        auto audioSys = std::make_unique<AudioSystem>();
        audioSys->init();
        audioSystem = audioSys.get(); // Cache pointer
        systems.push_back(std::move(audioSys));
        std::cout << "[Game] Audio system enabled" << std::endl;
    } else {
        std::cout << "[Game] Audio system disabled (no OpenAL device)" << std::endl;
    }

    systems.push_back(std::make_unique<EditorSystem>());
    systems.push_back(std::make_unique<AssetManagerWindow>());
    systems.push_back(std::make_unique<ModelEditorSystem>());

    // New ImGui windows - QuickActionsWindow should be first to draw menu bar
    systems.push_back(std::make_unique<QuickActionsWindow>());
    systems.push_back(std::make_unique<PerformanceWindow>());
    systems.push_back(std::make_unique<ConsoleWindow>());
    systems.push_back(std::make_unique<SceneHierarchyWindow>());
    systems.push_back(std::make_unique<SettingsWindow>());

    auto cs = std::make_unique<CollisionSystem>();
    collisionSystem = cs.get();
    systems.push_back(std::move(cs));

    std::cout << "ECS Systeme initialisiert" << std::endl;
}

void Game::setupEntities() {
    test_entity = entityManager.createEntity();
    test_entity->tag = "Player";
    test_entity->addComponent<TransformComponent>();
    test_entity->getComponent<TransformComponent>()->position = glm::vec2(960.0f, 540.0f);
    test_entity->getComponent<TransformComponent>()->scale = glm::vec2(100.0f, 100.0f);
    test_entity->addComponent<RenderComponent>();
    test_entity->getComponent<RenderComponent>()->setMesh("quad");
    test_entity->getComponent<RenderComponent>()->shaderName = "default";
    test_entity->getComponent<RenderComponent>()->color = glm::vec3(1.0f, 1.0f, 1.0f);
    test_entity->getComponent<RenderComponent>()->alpha = 1.0f;
    test_entity->getComponent<RenderComponent>()->renderLayer = 1;

    test_entity2 = entityManager.createEntity();
    test_entity2->tag = "Player2";
    test_entity2->addComponent<TransformComponent>();
    test_entity2->getComponent<TransformComponent>()->position = glm::vec2(1260.0f, 540.0f);
    test_entity2->getComponent<TransformComponent>()->scale = glm::vec2(100.0f, 100.0f);
    test_entity2->addComponent<RenderComponent>();
    test_entity2->getComponent<RenderComponent>()->setMesh("circle");
    test_entity2->getComponent<RenderComponent>()->shaderName = "default";
    test_entity2->getComponent<RenderComponent>()->color = glm::vec3(1.0f, 1.0f, 1.0f);
    test_entity2->getComponent<RenderComponent>()->alpha = 1.0f;
    test_entity2->getComponent<RenderComponent>()->renderLayer = 0;

    AssetManager::getInstance()->loadTexture("playerTexture", "res/textures/rotes_paddle.png");
    AssetManager::getInstance()->loadTexture("player2Texture", "res/textures/blaues_paddle.png");
    test_entity->getComponent<RenderComponent>()->textureName = "playerTexture";
    test_entity2->getComponent<RenderComponent>()->textureName = "player2Texture";

    std::cout << "ECS Entities initialisiert" << std::endl;
}

void Game::setupConfigs() {
}

void Game::onWindowResize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    
    // Update RenderSystem projection
    for (auto& system : systems) {
        if (auto* renderSystem = dynamic_cast<RenderSystem*>(system.get())) {
            renderSystem->setProjectionMatrix(glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f));
            break;
        }
    }
    
    // Update UIRenderer window size
    if (uiRenderer) {
        uiRenderer->updateWindowSize(width, height);
    }
}

