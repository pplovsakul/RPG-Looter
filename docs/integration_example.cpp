// Integration Example: Rendering cube.obj using the Mesh class
// This demonstrates how to integrate the OBJ/MTL parser with the existing rendering pipeline

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

// Add the Mesh and ObjParser includes
#include "Mesh.h"
#include "ObjParser.h"

// GLM für Matrizen
#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"

// ... [Keep all your existing camera and input code here] ...

int main(void) {
    // ... [Keep all your existing GLFW and OpenGL initialization code] ...

    // ===== LOAD MESH FROM OBJ FILE =====
    Mesh mesh;
    bool success = ObjParser::ParseOBJ("res/models/cube.obj", mesh, true);
    
    if (!success) {
        std::cerr << "Failed to load cube.obj!" << std::endl;
        return -1;
    }
    
    std::cout << "Mesh loaded successfully:" << std::endl;
    std::cout << "  - Vertices: " << mesh.GetVertexCount() << std::endl;
    std::cout << "  - Faces: " << mesh.GetFaceCount() << std::endl;
    std::cout << "  - Materials: " << mesh.GetMaterialCount() << std::endl;
    
    // Get interleaved vertex data (position, normal, texcoord)
    std::vector<float> vertexData = mesh.GetInterleavedVertexData();
    std::vector<unsigned int> indexData = mesh.GetIndexData();
    
    // Create OpenGL buffers from mesh data
    VertexBuffer vb(vertexData.data(), vertexData.size() * sizeof(float));
    IndexBuffer ib(indexData.data(), indexData.size() * sizeof(unsigned int));
    
    // Setup vertex array with proper layout for interleaved data
    VertexArray va;
    VertexBufferLayout layout;
    layout.AddFloat(3);  // Position (x, y, z)
    layout.AddFloat(3);  // Normal (nx, ny, nz)
    layout.AddFloat(2);  // Texture coordinates (u, v)
    va.AddBuffer(vb, layout);

    // ===== SHADER UND RENDERER SETUP =====
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
    
    // Model Matrix
    glm::mat4 model = glm::mat4(1.0f);

    // Main rendering loop
    while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        // ... [Keep your existing frame timing and input code] ...
        
        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View Matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        
        // MVP Matrix
        glm::mat4 mvp = projection * view * model;

        // ===== RENDERING THE LOADED MESH =====
        shader.Bind();
        shader.SetUniformMat4f("u_MVP", mvp);
        
        // You can set material colors here if needed
        // For example, use the first material's diffuse color:
        const Material* mat = mesh.GetMaterial("RedMaterial");
        if (mat) {
            shader.SetUniform4f("u_Color", 
                mat->diffuse[0], mat->diffuse[1], mat->diffuse[2], 1.0f);
        } else {
            shader.SetUniform4f("u_Color", 1.0f, 0.5f, 0.2f, 1.0f);
        }
        
        // Draw the loaded mesh
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
