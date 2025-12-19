# Player Class Usage Example

This document provides examples of how to use the Player class in the RPG-Looter project.

## Basic Setup

```cpp
#include "Player.h"
#include "Mesh.h"
#include "Shader.h"
#include "OBJLoader.h"

// Create a player at the origin
Player player;

// Or create a player at a specific position
Player player2(glm::vec3(10.0f, 0.0f, 5.0f));
```

## Mesh Assignment

```cpp
// Load mesh data from OBJ file
OBJLoader::MeshData meshData;
if (OBJLoader::LoadOBJ("res/models/player.obj", meshData)) {
    // Create a shared mesh that can be used by multiple players
    auto mesh = std::make_shared<Mesh>(meshData);
    
    // After OpenGL context is initialized and GLAD is loaded:
    mesh->SetupGL();
    
    // Assign mesh to player
    player.SetMesh(mesh);
}
```

## Input Handling

```cpp
// In your input handling code:
InputState input;
input.up    = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
input.down  = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
input.left  = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
input.right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
input.jump  = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

// Update player velocity based on input
player.HandleInput(input);
```

## Game Loop Integration

```cpp
// In your main game loop:
while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    
    // Handle input
    InputState input = getInputState(); // Your input gathering function
    player.HandleInput(input);
    
    // Update player (movement and physics)
    player.Update(deltaTime);
    
    // Render
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    shader.Bind();
    // Set view and projection matrices
    shader.SetUniformMat4f("u_View", view);
    shader.SetUniformMat4f("u_Projection", projection);
    
    // Draw player (this will set the model matrix internally)
    player.Draw(shader);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}
```

## Position and Movement Control

```cpp
// Get current position
glm::vec3 pos = player.GetPosition();
std::cout << "Player at: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;

// Set position gradually (will be affected by next Update call)
player.SetPosition(glm::vec3(5.0f, 0.0f, 0.0f));

// Teleport instantly to a position
player.Teleport(glm::vec3(100.0f, 50.0f, 25.0f));

// Adjust movement speed
player.SetSpeed(10.0f); // Faster movement
float currentSpeed = player.GetSpeed();
```

## Gravity

```cpp
// Enable gravity simulation
player.SetUseGravity(true);

// When gravity is enabled, the player will fall unless on ground
// You'll need to implement ground collision detection separately

// Disable gravity for flying mode
player.SetUseGravity(false);
```

## Multiple Players

```cpp
// Share the same mesh between multiple players
auto sharedMesh = std::make_shared<Mesh>(meshData);
sharedMesh->SetupGL();

Player player1(glm::vec3(-5.0f, 0.0f, 0.0f));
Player player2(glm::vec3( 5.0f, 0.0f, 0.0f));

player1.SetMesh(sharedMesh);
player2.SetMesh(sharedMesh);

// Each player has independent position and movement
// but they share the same mesh geometry (memory efficient)
```

## Shader Requirements

The Player class expects the shader to have the following uniform:
- `u_Model` (mat4): The model transformation matrix

Example shader setup:
```glsl
// Vertex Shader
#version 330 core
layout(location = 0) in vec3 a_Position;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}
```

## Notes

- The Player class does NOT handle health, damage, or healing mechanics
- Collision detection must be implemented separately
- The shader must be bound before calling `Draw()`
- The mesh must have `SetupGL()` called after OpenGL context is initialized
- Movement uses a simple velocity-based system - you can extend this with acceleration if needed
