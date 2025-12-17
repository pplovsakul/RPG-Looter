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

// Tastatur-Eingabe verarbeiten
void processInput(GLFWwindow* window) {
    float velocity = cameraSpeed * deltaTime;

    // WASD Bewegung
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += velocity * cameraFront;  // Vorw�rts
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= velocity * cameraFront;  // R�ckw�rts
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;  // Links
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;  // Rechts
    
    // Hoch/Runter (Space/Shift)
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += velocity * cameraUp;  // Hoch
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
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
    std::cout << "WASD    - Bewegen" << std::endl;
    std::cout << "Maus    - Umsehen" << std::endl;
    std::cout << "Space   - Hoch" << std::endl;
    std::cout << "Shift   - Runter" << std::endl;
    std::cout << "\nESC     - Beenden" << std::endl;

    // Maus einfangen und Callback setzen
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    
    // Enable depth testing for 3D
    glEnable(GL_DEPTH_TEST);

    // Set up a simple quad
    float vertices[] = {
        // Positions      
        -0.5f, -0.5f, 0.0f,  // Bottom left
         0.5f, -0.5f, 0.0f,  // Bottom right
        -0.5f,  0.5f, 0.0f,  // Top left
         0.5f,  0.5f, 0.0f,  // Top right

        -0.5f, -0.5f, 1.0f,
         0.5f, -0.5f, 1.0f,
        -0.5f,  0.5f, 1.0f,
         0.5f,  0.5f, 1.0f,

    };

    unsigned int indices[] = {
        0, 1, 2, // Triangle
        1, 3, 2,  // Triangle

		4, 5, 6,
		5, 7, 6,

		0, 1, 4,
		1, 5, 4,

		2, 3, 6,
		3, 7, 6,

		0, 2, 4,
		2, 6, 4,

		1, 3, 5,
		3, 7, 5
    };

    // ===== PLAUSIBILITY CHECK FOR INDEX DATA =====
    // Calculate the number of indices
    // NOTE: sizeof() works here because 'indices' is a local array, not a pointer parameter
    unsigned int indexCount = sizeof(indices) / sizeof(unsigned int);
    unsigned int vertexDataSize = sizeof(vertices);
    
    // Check 1: Ensure index data is not empty
    if (indexCount == 0) {
        std::cerr << "ERROR: Index data is empty! Cannot create IndexBuffer." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // Check 2: Ensure vertex data is not empty
    if (vertexDataSize == 0) {
        std::cerr << "ERROR: Vertex data is empty! Cannot create buffers." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // Check 3: Ensure index count doesn't exceed the maximum limit
    // This prevents uploading unreasonably large index buffers due to faulty mesh or OBJ parsing
    if (indexCount > MAX_INDEX_COUNT) {
        std::cerr << "ERROR: Index count (" << indexCount << ") exceeds maximum allowed (" 
                  << MAX_INDEX_COUNT << ")!" << std::endl;
        std::cerr << "This may indicate a faulty mesh or parsing error. Aborting." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    std::cout << "Mesh validation passed: " << indexCount << " indices, " 
              << vertexDataSize << " bytes of vertex data" << std::endl;

    // Create buffers
    VertexBuffer vb(vertices, sizeof(vertices));
    IndexBuffer ib(indices, indexCount);
    
    VertexArray va;
    VertexBufferLayout layout;
    layout.AddFloat(3); // Position attribute (3 floats: x, y, z)
    va.AddBuffer(vb, layout);

    // ===== SHADER UND RENDERER SETUP =====
    // basic.shader: Standard OpenGL Vertex/Fragment Shader für Rasterizer
    Shader shader("res/shaders/basic.shader");

    Renderer renderer;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Projection Matrix (Perspective)
    float aspectRatio = (float)windowWidth / (float)windowHeight;
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        aspectRatio,
        0.1f,
        100.0f
    );
    
    // Model Matrix (Objekt bleibt am Ursprung)
    glm::mat4 model = glm::mat4(1.0f);

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
            std::string title = "3D Camera Demo - WASD + Maus - FPS: " + std::to_string(static_cast<int>(fps));
            glfwSetWindowTitle(window, title.c_str());
            lastTitleUpdate = currentTime;
        }

        // Eingabe verarbeiten
        processInput(window);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View Matrix aus Kamera-Variablen berechnen
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        
        // MVP Matrix berechnen
        glm::mat4 mvp = projection * view * model;

        // ===== RENDERING =====
        // Standard OpenGL GPU-Rendering mit Vertex/Fragment Shadern
        shader.Bind();
        shader.SetUniformMat4f("u_MVP", mvp);
        shader.SetUniform4f("u_Color", 1.0f, 0.5f, 0.2f, 1.0f);
        renderer.Draw(va, ib, shader);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
