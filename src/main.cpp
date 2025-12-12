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
#include "GPURayTracer.h"
#include "Material.h"

// GLM für Matrizen
#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"

int width = 1280, height = 720;

// ===== RAY TRACER KONFIGURATION =====
// GPU Ray Tracer: Kann volle Auflösung nutzen
const int GPU_RT_WIDTH = 1280;
const int GPU_RT_HEIGHT = 720;

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

// ===== RAY TRACER HOTKEY STATE =====
// Tracking für Hotkeys mit Debouncing
bool rKeyWasPressed = false;
bool key1WasPressed = false;
bool key2WasPressed = false;
bool key3WasPressed = false;
bool key4WasPressed = false;
bool bKeyWasPressed = false;
bool mKeyWasPressed = false;

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
    std::cout << "R       - Toggle Ray Tracer (Rasterizer/GPU Ray Tracer)" << std::endl;
    std::cout << "1-4     - Samples per Pixel (1, 4, 9, 16)" << std::endl;
    std::cout << "B       - Bounce Depth erhöhen (max 10)" << std::endl;
    std::cout << "M       - Material Set wechseln" << std::endl;
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
    // Rendering-Modi: 0 = Rasterizer, 1 = GPU Ray Tracer
    int renderMode = 0;
    
    // basic.shader: Standard OpenGL Vertex/Fragment Shader für Rasterizer
    Shader shader("res/shaders/basic.shader");
    
    // neuer_shader.shader: Shader zum Anzeigen der Ray-Traced Textur auf einem Fullscreen Quad
    Shader rtshader("res/shaders/neuer_shader.shader");
    
    // GPU Ray Tracer: Volle Auflösung (1280x720)
    GPURayTracer* gpuRT = nullptr;
    bool gpuRTAvailable = false;
    
    // Prüfe ob OpenGL 4.3+ verfügbar ist (für Compute Shaders)
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    bool computeShadersSupported = (major > 4) || (major == 4 && minor >= 3);
    
    if (computeShadersSupported) {
        try {
            gpuRT = new GPURayTracer(GPU_RT_WIDTH, GPU_RT_HEIGHT);
            if (gpuRT->isAvailable()) {
                gpuRTAvailable = true;
                std::cout << "\nGPU Ray Tracer verfügbar! (OpenGL " << major << "." << minor << ")" << std::endl;
            } else {
                delete gpuRT;
                gpuRT = nullptr;
            }
        } catch (...) {
            std::cerr << "GPU Ray Tracer konnte nicht initialisiert werden." << std::endl;
            if (gpuRT) {
                delete gpuRT;
                gpuRT = nullptr;
            }
        }
    } else {
        std::cout << "\nGPU Ray Tracer nicht verfügbar (OpenGL " << major << "." << minor << " < 4.3)" << std::endl;
        std::cout << "CPU Ray Tracer wird als Fallback verwendet." << std::endl;
    }
    
    // ===== SZENE KONFIGURATION =====
    // Raumgröße: 5x5x5 Einheiten, Zentrum bei (0, 0, 2.5)
    // Wanddicke: 0.2 Einheiten
    const float roomSize = 5.0f;
    const float wallThickness = 0.2f;
    Material boxMaterial = Material::Diffuse(glm::vec3(0.8f, 0.3f, 0.3f));
    Material wallMaterial = Material::Diffuse(glm::vec3(0.7f, 0.7f, 0.7f)); // Hellgrau
    Material lampMaterial = Material::Emissive(glm::vec3(1.0f, 1.0f, 0.9f), 20.0f); // Warmes weißes Licht, erhöhte Intensität
    
    if (gpuRT) {
        // Füge Würfel zum Ray Tracer hinzu (Center: (0, 0, 0.5), Size: (1.0, 1.0, 1.0))
        gpuRT->boxes.emplace_back(Box::fromCenterSize(glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), boxMaterial));
        
        // ===== RAUM MIT OFFENER FRONTWAND =====
        // Boden (unten)
        gpuRT->boxes.emplace_back(Box::fromCenterSize(
            glm::vec3(0.0f, -roomSize/2.0f - wallThickness/2.0f, roomSize/2.0f),
            glm::vec3(roomSize, wallThickness, roomSize),
            wallMaterial
        ));
        
        // Decke (oben)
        gpuRT->boxes.emplace_back(Box::fromCenterSize(
            glm::vec3(0.0f, roomSize/2.0f + wallThickness/2.0f, roomSize/2.0f),
            glm::vec3(roomSize, wallThickness, roomSize),
            wallMaterial
        ));
        
        // Linke Wand
        gpuRT->boxes.emplace_back(Box::fromCenterSize(
            glm::vec3(-roomSize/2.0f - wallThickness/2.0f, 0.0f, roomSize/2.0f),
            glm::vec3(wallThickness, roomSize, roomSize),
            wallMaterial
        ));
        
        // Rechte Wand
        gpuRT->boxes.emplace_back(Box::fromCenterSize(
            glm::vec3(roomSize/2.0f + wallThickness/2.0f, 0.0f, roomSize/2.0f),
            glm::vec3(wallThickness, roomSize, roomSize),
            wallMaterial
        ));
        
        // Rückwand
        gpuRT->boxes.emplace_back(Box::fromCenterSize(
            glm::vec3(0.0f, 0.0f, roomSize + wallThickness/2.0f),
            glm::vec3(roomSize, roomSize, wallThickness),
            wallMaterial
        ));
        
        // FRONTWAND WIRD NICHT HINZUGEFÜGT - der Raum ist auf dieser Seite offen!
        
        // ===== DECKENLAMPE (EINZIGE LICHTQUELLE) =====
        // Leuchtende Kugel an der Decke - größer und heller für bessere Raumbeleuchtung
        gpuRT->spheres.emplace_back(Sphere(
            glm::vec3(0.0f, roomSize/2.0f, roomSize/2.0f),
            0.5f,
            lampMaterial
        ));
    }

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

        // ===== HOTKEY HANDLING MIT DEBOUNCING =====
        
        // R-Taste: Toggle zwischen Rasterizer / GPU Ray Tracer
        bool rKeyIsPressed = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
        if (rKeyIsPressed && !rKeyWasPressed) {
            renderMode = (renderMode + 1) % (gpuRTAvailable ? 2 : 1);
            const char* modes[] = { "Rasterizer", "GPU Ray Tracer" };
            std::cout << "Switched to " << modes[renderMode] << " mode" << std::endl;
        }
        rKeyWasPressed = rKeyIsPressed;
        
        // 1-4: Samples per Pixel
        bool key1IsPressed = (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS);
        if (key1IsPressed && !key1WasPressed) {
            if (gpuRT) gpuRT->samplesPerPixel = 1;
            std::cout << "Samples per Pixel: 1" << std::endl;
        }
        key1WasPressed = key1IsPressed;
        
        bool key2IsPressed = (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS);
        if (key2IsPressed && !key2WasPressed) {
            if (gpuRT) gpuRT->samplesPerPixel = 4;
            std::cout << "Samples per Pixel: 4" << std::endl;
        }
        key2WasPressed = key2IsPressed;
        
        bool key3IsPressed = (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS);
        if (key3IsPressed && !key3WasPressed) {
            if (gpuRT) gpuRT->samplesPerPixel = 9;
            std::cout << "Samples per Pixel: 9" << std::endl;
        }
        key3WasPressed = key3IsPressed;
        
        bool key4IsPressed = (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS);
        if (key4IsPressed && !key4WasPressed) {
            if (gpuRT) gpuRT->samplesPerPixel = 16;
            std::cout << "Samples per Pixel: 16" << std::endl;
        }
        key4WasPressed = key4IsPressed;
        
        // B: Bounce Depth erhöhen
        bool bKeyIsPressed = (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS);
        if (bKeyIsPressed && !bKeyWasPressed) {
            if (gpuRT) {
                gpuRT->maxBounces = (gpuRT->maxBounces % 10) + 1;
                std::cout << "Max Bounces: " << gpuRT->maxBounces << std::endl;
            }
        }
        bKeyWasPressed = bKeyIsPressed;
        
        // M: Material Set wechseln
        bool mKeyIsPressed = (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS);
        if (mKeyIsPressed && !mKeyWasPressed) {
            if (gpuRT) gpuRT->cycleMaterialSet();
        }
        mKeyWasPressed = mKeyIsPressed;

        // Eingabe verarbeiten
        processInput(window);

        // ===== KAMERA SYNCHRONISATION =====
        // Die Kamera-Controls (WASD, Maus) aktualisieren globale Variablen (cameraPos, cameraFront, etc.)
        if (gpuRT) {
            gpuRT->camera.position = cameraPos;
            gpuRT->camera.target = cameraPos + cameraFront;
            gpuRT->camera.up = cameraUp;
            gpuRT->camera.vfov = 45.0f;
        }

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View Matrix aus Kamera-Variablen berechnen
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        
        // MVP Matrix berechnen
        glm::mat4 mvp = projection * view * model;

        // ===== RENDERING BASIEREND AUF MODUS =====
        if (renderMode == 1 && gpuRT) {
            // GPU RAY TRACER MODUS:
            // 1. Compute Shader berechnet das Bild parallel auf der GPU
            // 2. Ergebnis wird direkt in eine Textur geschrieben
            // 3. Textur wird auf einem Fullscreen-Quad angezeigt
            // VORTEIL: Viel schneller, höhere Auflösung möglich (1280x720)
            gpuRT->draw(rtshader.GetRendererID());
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
    if (gpuRT) delete gpuRT;
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
