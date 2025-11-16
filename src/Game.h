#include "Entitymanager.h"
#include "System.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Game {
private:
	EntityManager entityManager;
	std::vector<std::unique_ptr<System>> systems;
	Entity* test_entity;
	Entity* test_entity2;

	void setupSystems(GLFWwindow* window);
	void setupEntities();
	void setupConfigs();

public:
	void setup(GLFWwindow* window);
	void update(float deltaTime);
	EntityManager& getEntityManager() { return entityManager; }
	void onWindowResize(GLFWwindow* window, int width, int height);
};