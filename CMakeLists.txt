cmake_minimum_required(VERSION 3.13)

project(RPG-Looter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
add_subdirectory("extern/glfw-3.4")

# Executable
add_executable(RPG-Looter
    "src/Debug.h"
    "src/Renderer.cpp"
    "src/Renderer.h"
    "src/VertexBuffer.h"
    "src/VertexBuffer.cpp"
    "src/Shader.h"
    "src/Shader.cpp"
    "src/VertexArray.h"
    "src/VertexArray.cpp"
    "src/VertexBufferLayout.h"
    "src/VertexBufferLayout.cpp"
    "src/IndexBuffer.h"
    "src/IndexBuffer.cpp"
    "src/InputSystem.h"
    
    # GLAD
    "src/vendor/glad/glad.c"
    
    # GLM Header
    "src/vendor/glm/glm.hpp"
    
    "src/main.cpp"
 "src/Ray.h" "src/Hit.h" "src/Sphere.h" "src/Box.h" "src/Camera.h" "src/RayTraceRenderer.h" "src/RayTracer.h")

# Includes
target_include_directories(RPG-Looter PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)

# Link GLFW
target_link_libraries(RPG-Looter PRIVATE glfw)

# Copy shaders (res/shaders -> build/res/shaders)
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders")
set(SHADER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/res/shaders")

file(GLOB SHADER_FILES "${SHADER_SOURCE_DIR}/*.shader")

foreach(SHADER_FILE ${SHADER_FILES})
    configure_file(${SHADER_FILE} ${SHADER_BUILD_DIR}/ COPYONLY)
endforeach()

target_compile_definitions(RPG-Looter PRIVATE SHADER_DIR="${SHADER_BUILD_DIR}")
