#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "InputSystem.h"
#include "BufferLimits.h"
#include "OBJLoader.h"
#include "Mesh.h"
#include "Player.h"

// GLM für Matrizen
#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"

int width = 1280, height = 720;

// ===== KAMERA VARIABLEN =====
// Kamera Position und Orientierung
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, -3.0f);  // Außerhalb des Raums, vor der offenen Frontwand
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);    // Blick in den Raum hinein (positive Z-Richtung)
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

// Maus-Steuerung
float yaw   = 90.0f;   // Horizontale Rotation (links/rechts) - 90° für Blick in positive Z-Richtung
float pitch = 0.0f;    // Vertikale Rotation (hoch/runter)
float lastX = 640.0f;  // Letzte Maus X-Position
float lastY = 360.0f;  // Letzte Maus Y-Position
bool firstMouse = true;
float mouseSensitivity = 0.1f;

// Bewegungsgeschwindigkeit
float cameraSpeed = 2.5f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ===== FPS COUNTER =====
float lastTitleUpdate = 0.0f;

// Maus-Callback Funktion
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Umgekehrt, da Y-Koordinaten von unten nach oben gehen
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    // Pitch begrenzen, damit Kamera nicht �berkippt
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Neue Blickrichtung berechnen
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Tastatur-Eingabe verarbeiten (Camera)
void processInput(GLFWwindow* window) {
    float velocity = cameraSpeed * deltaTime;

    // Arrow keys for camera movement (to avoid conflict with player WASD)
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        cameraPos += velocity * cameraFront;  // Vorwärts
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        cameraPos -= velocity * cameraFront;  // Rückwärts
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;  // Links
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;  // Rechts
    
    // Q/E for vertical camera movement
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPos += velocity * cameraUp;  // Hoch
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraPos -= velocity * cameraUp;  // Runter
}

int main(void) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Set OpenGL version hints - Use 4.3 for compute shader support
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    const int windowWidth = 1280;
    const int windowHeight = 720;
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "3D Camera Demo - WASD + Maus", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Enable vsync

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "\n=== STEUERUNG ===" << std::endl;
    std::cout << "WASD        - Player bewegen" << std::endl;
    std::cout << "Arrow Keys  - Kamera bewegen" << std::endl;
    std::cout << "Q/E         - Kamera hoch/runter" << std::endl;
    std::cout << "Maus        - Kamera umsehen" << std::endl;
    std::cout << "\nESC         - Beenden" << std::endl;

    // Maus einfangen und Callback setzen
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    
    // Enable depth testing for 3D
    glEnable(GL_DEPTH_TEST);

    // ===== LOAD MESH FROM OBJ FILE =====

    OBJLoader::MeshData staticData;
    if (OBJLoader::LoadOBJ("res/models/Test.obj", staticData)) {
        auto staticMesh = std::make_shared<Mesh>(staticData);
        staticMesh->SetupGL();

        // Position des Objekts festlegen
        glm::vec3 objectPos = glm::vec3(5.0f, 0.0f, -5.0f);
    }

    std::cout << "\n=== Loading Player Modell ===" << std::endl;
    OBJLoader::MeshData playerData;
	char ModelPath[] = "res/models/Test.obj";
    if (!OBJLoader::LoadOBJ(ModelPath, playerData)) {
        std::cerr << "ERROR: Failed to load" << ModelPath <<"!" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Get mesh data
    std::shared_ptr<Mesh> meshPtr = std::make_shared<Mesh>(playerData);;
	meshPtr->SetupGL();

	std::shared_ptr<Mesh> meshPtr2 = std::make_shared<Mesh>(staticData);
	meshPtr2->SetupGL();
    
    std::cout << "Mesh loaded successfully!" << std::endl;
   
    
    // Create player at origin
    Player player(glm::vec3(0.0f, 0.0f, 0.0f));
    player.SetMesh(meshPtr);
    player.SetSpeed(2.5f);

	Player well(glm::vec3(5.0f, 0.0f, -5.0f));
	well.SetMesh(meshPtr2);
    
    std::cout << "Player created with mesh" << std::endl;

    // ===== SHADER UND RENDERER SETUP =====
    // basic.shader: Standard OpenGL Vertex/Fragment Shader für Rasterizer
    Shader shader("res/shaders/basic.shader");

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    // Check if we have a texture from the loaded materials for player
    bool usePlayerTexture = false;
    std::shared_ptr<Texture> playerTexture = nullptr;
    
    if (!playerData.materials.empty()) {
        // Use the first material with a texture
        for (const auto& matPair : playerData.materials) {
            if (matPair.second.diffuseTexture && matPair.second.diffuseTexture->IsValid()) {
                playerTexture = matPair.second.diffuseTexture;
                usePlayerTexture = true;
                std::cout << "Using texture from player material: " << matPair.first << std::endl;
                break;
            }
        }
    }
    
    if (!usePlayerTexture) {
        std::cout << "No texture found in player materials, using per-vertex material colors." << std::endl;
        if (playerData.hasVertexColors) {
            std::cout << "Mesh has per-vertex colors assigned from material diffuse colors (Kd)." << std::endl;
        } else {
            std::cout << "Warning: No vertex colors available - mesh will use default white color." << std::endl;
        }
    }
    
    // Check if we have a texture from the loaded materials for well (staticData)
    bool useWellTexture = false;
    std::shared_ptr<Texture> wellTexture = nullptr;
    
    if (!staticData.materials.empty()) {
        // Use the first material with a texture
        for (const auto& matPair : staticData.materials) {
            if (matPair.second.diffuseTexture && matPair.second.diffuseTexture->IsValid()) {
                wellTexture = matPair.second.diffuseTexture;
                useWellTexture = true;
                std::cout << "Using texture from well material: " << matPair.first << std::endl;
                break;
            }
        }
    }
    
    if (!useWellTexture) {
        std::cout << "No texture found in well materials, using per-vertex material colors." << std::endl;
        if (staticData.hasVertexColors) {
            std::cout << "Well mesh has per-vertex colors assigned from material diffuse colors (Kd)." << std::endl;
        } else {
            std::cout << "Warning: Well has no vertex colors available - mesh will use default white color." << std::endl;
        }
    }

    // Projection Matrix (Perspective)
    float aspectRatio = (float)windowWidth / (float)windowHeight;
    glm::mat4 projection = glm::perspective(

        glm::radians(45.0f),
        aspectRatio,
        0.1f,
        100.0f
    );

	glEnable(GL_FRAMEBUFFER_SRGB); //Copilot , dont delete. Enable sRGB for correct color space

    // Main loop
    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        // Delta Time berechnen
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // ===== FPS COUNTER UPDATE =====
        float currentTime = static_cast<float>(glfwGetTime());
        if (currentTime - lastTitleUpdate >= 0.5f) {
            float fps = 1.0f / deltaTime;
            std::string title = "Player Test - WASD + Maus - FPS: " + std::to_string(static_cast<int>(fps));
            glfwSetWindowTitle(window, title.c_str());
            lastTitleUpdate = currentTime;
        }

        // Eingabe verarbeiten (Camera)
        processInput(window);
        
        // Player Input handling
        InputState playerInput;
        playerInput.up    = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        playerInput.down  = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        playerInput.left  = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        playerInput.right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
        playerInput.jump  = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        
        player.HandleInput(playerInput);
        player.Update(deltaTime);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View Matrix aus Kamera-Variablen berechnen
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // ===== RENDERING =====
        shader.Bind();
        
        // Set view and projection matrices
        shader.SetUniformMat4f("u_View", view);
        shader.SetUniformMat4f("u_Projection", projection);
        
        // Draw player with its texture state
        if (usePlayerTexture && playerTexture) {
            playerTexture->Bind(0); // Bind to texture slot 0
            shader.SetUniform1i("u_Texture", 0);
            shader.SetUniform1i("u_UseTexture", 1);
        } else {
            // Use per-vertex material colors
            shader.SetUniform1i("u_UseTexture", 0);
        }
        
        // Draw player (this will set u_Model matrix internally)
        player.Draw(shader);

        // Draw well with its texture state
        if (useWellTexture && wellTexture) {
            wellTexture->Bind(0); // Bind to texture slot 0
            shader.SetUniform1i("u_Texture", 0);
            shader.SetUniform1i("u_UseTexture", 1);
        } else {
            // Use per-vertex material colors for well
            shader.SetUniform1i("u_UseTexture", 0);
        }

        well.Draw(shader);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
