cmake_minimum_required(VERSION 3.13)

project(RPG-Looter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
add_subdirectory("extern/glfw-3.4")

# Executable
add_executable(RPG-Looter
    "src/Debug.h"
    "src/BufferLimits.h"
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
    "src/OBJLoader.h"
    "src/OBJLoader.cpp"
    "src/Texture.h"
    "src/Texture.cpp"
    "src/Material.h"
    
    # OBJ and MTL parsers
    "include/obj/types.h"
    "include/obj/objparser.h"
    "src/objparser.cpp"
    "include/obj/mtlparser.h"
    "src/mtlparser.cpp"
    "include/sig/sigslot.h"
    
    # GLAD
    "src/vendor/glad/glad.c"
    
    # GLM Header
    "src/vendor/glm/glm.hpp"
    
    # stb_image Header
    "src/vendor/stb/stb_image.h"
    
    "src/main.cpp"
 "src/Mesh.h" "src/Mesh.cpp" "include/Player.h" "src/Player.cpp"
 
 # Collision Detection System
 "src/AABB.h"
 "src/Triangle.h"
 "src/Octree.h"
 "src/Octree.cpp"
 "src/CollisionSystem.h"
 "src/CollisionSystem.cpp"
 "src/AABBRenderer.h"
 "src/AABBRenderer.cpp")

# Includes
target_include_directories(RPG-Looter PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/"
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

# Copy models (res/models -> build/res/models)
set(MODELS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/models")
set(MODELS_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/res/models")

if(EXISTS ${MODELS_SOURCE_DIR})
    file(GLOB MODEL_FILES "${MODELS_SOURCE_DIR}/*")
    file(MAKE_DIRECTORY ${MODELS_BUILD_DIR})
    
    foreach(MODEL_FILE ${MODEL_FILES})
        get_filename_component(MODEL_NAME ${MODEL_FILE} NAME)
        configure_file(${MODEL_FILE} ${MODELS_BUILD_DIR}/${MODEL_NAME} COPYONLY)
    endforeach()
endif()

target_compile_definitions(RPG-Looter PRIVATE SHADER_DIR="${SHADER_BUILD_DIR}")
