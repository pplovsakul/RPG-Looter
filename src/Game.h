#include "Entitymanager.h"
#include "System.h"
#include "GlobalSettings.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Forward declarations to avoid including heavy headers
class CollisionSystem;
class RenderSystem;
class AudioSystem;

class Game {
private:
	EntityManager entityManager;
	std::vector<std::unique_ptr<System>> systems;
	Entity* test_entity;
	Entity* test_entity2;
	
	// Cache pointers to avoid repeated dynamic_cast
	CollisionSystem* collisionSystem = nullptr;
	RenderSystem* renderSystem = nullptr;
	AudioSystem* audioSystem = nullptr;
    bool audioAvailable = false; // flag to control optional audio system
	
	GLFWwindow* gameWindow = nullptr; // Store window for VSync control

	void setupSystems(GLFWwindow* window);
	void setupEntities();
	void setupConfigs();

public:
	Game() = default;
	~Game() = default;
	
	void setup(GLFWwindow* window);
	void update(float deltaTime);
	EntityManager& getEntityManager() { return entityManager; }
	void onWindowResize(GLFWwindow* window, int width, int height);
    void setAudioAvailable(bool available); // setter
	void applyVSync(); // Apply VSync settings
	GLFWwindow* getWindow() { return gameWindow; }
};