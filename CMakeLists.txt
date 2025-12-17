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
