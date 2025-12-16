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
    
    # Mesh and Material System
    "src/Material.h"
    "src/Mesh.h"
    "src/Mesh.cpp"
    "src/ObjParser.h"
    "src/ObjParser.cpp"
    
    # GLAD
    "src/vendor/glad/glad.c"
    
    # GLM Header
    "src/vendor/glm/glm.hpp"
    
    "src/main.cpp"
)

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

# Create shader directory if it doesn't exist
file(MAKE_DIRECTORY ${SHADER_BUILD_DIR})

foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    configure_file(${SHADER_FILE} ${SHADER_BUILD_DIR}/${SHADER_NAME} COPYONLY)
endforeach()

target_compile_definitions(RPG-Looter PRIVATE SHADER_DIR="${SHADER_BUILD_DIR}")

# Test executable
add_executable(test_mesh
    "tests/test_mesh.cpp"
    "src/Mesh.cpp"
    "src/Mesh.h"
    "src/Material.h"
    "src/ObjParser.cpp"
    "src/ObjParser.h"
)

# Copy test resources (res/models -> build/res/models)
set(MODEL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/models")
set(MODEL_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/res/models")

file(GLOB MODEL_FILES "${MODEL_SOURCE_DIR}/*")

# Create model directory if it doesn't exist
file(MAKE_DIRECTORY ${MODEL_BUILD_DIR})

foreach(MODEL_FILE ${MODEL_FILES})
    get_filename_component(MODEL_NAME ${MODEL_FILE} NAME)
    configure_file(${MODEL_FILE} ${MODEL_BUILD_DIR}/${MODEL_NAME} COPYONLY)
endforeach()
