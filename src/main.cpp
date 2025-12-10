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
#include "RayTraceRenderer.h"

// GLM f�r Matrizen
#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"

int width = 1280, height = 720;

// ===== RAY TRACER KONFIGURATION =====
// Ray Tracer Resolution: Reduziert auf 400x300, um CPU-Last zu minimieren
// und Einfrieren während des Renderns zu vermeiden. Bei höheren Auflösungen
// kann das Ray Tracing mehrere Sekunden pro Frame dauern.
const int RT_WIDTH = 400;
const int RT_HEIGHT = 300;

// ===== KAMERA VARIABLEN =====
// Kamera Position und Orientierung
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

// Maus-Steuerung
float yaw   = -90.0f;  // Horizontale Rotation (links/rechts)
float pitch = 0.0f;    // Vertikale Rotation (hoch/runter)
float lastX = 640.0f;  // Letzte Maus X-Position
float lastY = 360.0f;  // Letzte Maus Y-Position
bool firstMouse = true;
float mouseSensitivity = 0.1f;

// Bewegungsgeschwindigkeit
float cameraSpeed = 2.5f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ===== RAY TRACER HOTKEY STATE =====
// Tracking für R-Taste, um sauberes Toggle zwischen Rasterizer/Ray Tracer zu ermöglichen
// Ohne Debouncing würde die Taste bei jedem Frame mehrfach erkannt werden
bool rKeyWasPressed = false;

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

    // Set OpenGL version hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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
    glfwSwapInterval(1); // Enable vsync

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
    std::cout << "R       - Toggle Ray Tracer / Rasterizer" << std::endl;
    std::cout << "ESC     - Beenden" << std::endl;

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

    // Create buffers
    VertexBuffer vb(vertices, sizeof(vertices));
    IndexBuffer ib(indices, sizeof(indices));
    
    VertexArray va;
    VertexBufferLayout layout;
    layout.AddFloat(3); // Position attribute (3 floats: x, y, z)
    va.AddBuffer(vb, layout);

    // ===== SHADER UND RENDERER SETUP =====
    // useRayTracer: Flag zum Umschalten zwischen Rasterizer (OpenGL) und Ray Tracer (CPU)
	bool useRayTracer = false;
    
    // basic.shader: Standard OpenGL Vertex/Fragment Shader für Rasterizer
	Shader shader("res/shaders/basic.shader");
    
    // neuer_shader.shader: Shader zum Anzeigen der Ray-Traced Textur auf einem Fullscreen Quad
    Shader rtshader("res/shaders/neuer_shader.shader");
    
    // RayTraceRenderer: CPU-basierter Ray Tracer mit reduzierter Auflösung (400x300)
    // Dies verhindert Einfrieren, da Ray Tracing sehr rechenintensiv ist
    RayTraceRenderer rt(RT_WIDTH, RT_HEIGHT);
    
    // ===== SZENE KONFIGURATION =====
    // Füge Würfel aus der Rasterizer-Szene zum Ray Tracer hinzu
    // Der Würfel ist am Ursprung (0,0,0) mit Größe 1x1x1
    // Box::fromCenterSize erstellt eine AABB (Axis-Aligned Bounding Box) aus Zentrum und Größe
    rt.tracer.boxes.emplace_back(Box::fromCenterSize(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));

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

        // ===== HOTKEY TOGGLE MIT DEBOUNCING =====
        // R-Taste zum Umschalten zwischen Rasterizer und Ray Tracer
        // Debouncing: Nur bei Tastendruck-Flanke (nicht gedrückt -> gedrückt) togglen,
        // um mehrfaches Umschalten pro Frame zu verhindern
        bool rKeyIsPressed = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
        if (rKeyIsPressed && !rKeyWasPressed) {
            useRayTracer = !useRayTracer;
            std::cout << "Switched to " << (useRayTracer ? "Ray Tracer" : "Rasterizer") << " mode" << std::endl;
        }
        rKeyWasPressed = rKeyIsPressed;

        // Eingabe verarbeiten
        processInput(window);

        // ===== KAMERA SYNCHRONISATION =====
        // Die Kamera-Controls (WASD, Maus) aktualisieren globale Variablen (cameraPos, cameraFront, etc.)
        // Diese werden hier zum Ray Tracer synchronisiert, sodass beide Renderer dieselbe Ansicht haben
        rt.tracer.camera.position = cameraPos;
        rt.tracer.camera.target = cameraPos + cameraFront;
        rt.tracer.camera.up = cameraUp;
        rt.tracer.camera.vfov = 45.0f; // FOV muss mit dem Rasterizer übereinstimmen

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View Matrix aus Kamera-Variablen berechnen
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        
        // MVP Matrix berechnen
        glm::mat4 mvp = projection * view * model;

        // ===== RENDERING BASIEREND AUF MODUS =====
        if (useRayTracer) {
            // RAY TRACER MODUS:
            // 1. CPU berechnet das Bild Pixel für Pixel (siehe RayTracer::render())
            // 2. Bild wird als OpenGL-Textur hochgeladen
            // 3. Textur wird auf einem Fullscreen-Quad angezeigt
            // HINWEIS: Ray Tracing ist CPU-intensiv, daher niedrigere Auflösung (400x300)
            rt.draw(rtshader.GetRendererID());
        }
        else {
            // RASTERIZER MODUS:
            // Standard OpenGL GPU-Rendering mit Vertex/Fragment Shadern
            // Viel schneller als Ray Tracing, aber weniger physikalisch korrekt
            shader.Bind();
            shader.SetUniformMat4f("u_MVP", mvp);
            shader.SetUniform4f("u_Color", 1.0f, 0.5f, 0.2f, 1.0f);
            renderer.Draw(va, ib, shader);
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
