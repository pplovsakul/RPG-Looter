#include "Game.h"

#include "InputSystem.h"
#include "RenderSystem.h"
#include "AudioSystem.h"
#include "EditorSystem.h"
#include "AssetManagerWindow.h"
#include "ModelEditorSystem.h"
#include "CollisionSystem.h" // added

#include "Components.h"

#include "vendor/glm/gtc/matrix_transform.hpp"

void Game::update(float deltaTime) {
    for (auto& system : systems) {
        system->update(entityManager, deltaTime);
    }
    entityManager.cleanup();

    if (collisionSystem) {
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
    setupSystems(window);
    setupEntities();
}

void Game::setupSystems(GLFWwindow* window) {
    // Input System
    systems.push_back(std::make_unique<InputSystem>(window));

    // Render System
    auto renderSystem = std::make_unique<RenderSystem>();
    renderSystem->init();
    renderSystem->setViewMatrix(glm::mat4(1.0f));

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    renderSystem->setProjectionMatrix(glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f));
    systems.push_back(std::move(renderSystem));

    // Optional Audio System
    if (audioAvailable) {
        auto audioSys = std::make_unique<AudioSystem>();
        audioSys->init();
        systems.push_back(std::move(audioSys));
        std::cout << "[Game] Audio system enabled" << std::endl;
    } else {
        std::cout << "[Game] Audio system disabled (no OpenAL device)" << std::endl;
    }

    systems.push_back(std::make_unique<EditorSystem>());
    systems.push_back(std::make_unique<AssetManagerWindow>());
    systems.push_back(std::make_unique<ModelEditorSystem>());

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
    for (auto& system : systems) {
        if (auto* renderSystem = dynamic_cast<RenderSystem*>(system.get())) {
            renderSystem->setProjectionMatrix(glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f));
            break;
        }
    }
}

