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
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Minimal Graphics Framework", NULL, NULL);
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

    // Enable input handling
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Set up a simple triangle
    float vertices[] = {
        // Positions      
        -0.5f, -0.5f, 0.0f,  // Bottom left
         0.5f, -0.5f, 0.0f,  // Bottom right
         0.0f,  0.5f, 0.0f   // Top
    };

    unsigned int indices[] = {
        0, 1, 2  // Triangle
    };

    // Create buffers
    VertexBuffer vb(vertices, sizeof(vertices));
    IndexBuffer ib(indices, 3);
    
    VertexArray va;
    VertexBufferLayout layout;
    layout.AddFloat(3); // Position attribute
    va.AddBuffer(vb, layout);

    // Load shader
    Shader shader("res/shaders/basic.shader");
    
    Renderer renderer;

    // Initialize input system
    InputSystem inputSystem(window);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Main loop
    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        // Clear screen
        renderer.Clear();

        // Draw triangle
        shader.Bind();
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
