cmake_minimum_required(VERSION 3.13)

project(Pong)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW einbinden
add_subdirectory("extern/glfw-3.4")

# Executable definieren
add_executable(Pong 
# Hauptdatei
    "src/main.cpp"

# Gameplay Dateien
    "src/GameLogic.cpp"
    "src/GameLogic.h"
    "src/Debug.h"

# Menü Dateien
    "src/Menu.cpp"
    "src/Menu.h"
# Networking Dateien

# Rendering Dateien "GameRendering-alle-includes.txt"
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
    "src/Rendermanager.h" 
    "src/Rendermanager.cpp"

# GLAD Dateien
    "src/vendor/glad/glad.c"

# ImGui Dateien
    "src/vendor/imgui/imconfig.h"
    "src/vendor/imgui/imgui.cpp"
    "src/vendor/imgui/imgui.h"
    "src/vendor/imgui/imgui_demo.cpp"
    "src/vendor/imgui/imgui_draw.cpp"
    "src/vendor/imgui/imgui_tables.cpp"
    "src/vendor/imgui/imgui_widgets.cpp"
    "src/vendor/imgui/imgui_internal.h"
    "src/vendor/imgui/imgui_impl_glfw.cpp"
    "src/vendor/imgui/imgui_impl_glfw.h"
    "src/vendor/imgui/imgui_impl_opengl3.cpp"
    "src/vendor/imgui/imgui_impl_opengl3.h"
    "src/vendor/imgui/imgui_impl_opengl3_loader.h"
    "src/vendor/imgui/imstb_rectpack.h"
    "src/vendor/imgui/imstb_textedit.h"
    "src/vendor/imgui/imstb_truetype.h"

# GLM Header
    "src/vendor/glm/glm.hpp"

# Texture Loading Dateien
    "src/vendor/stb_image/stb_image.h"
    "src/vendor/stb_image/stb_image.cpp"
    "src/Texture.cpp" 
    "src/Texture.h"

# Sound
    "src/Audiomanager.h"
    "src/Audiomanager.cpp"
  )


# Includes
target_include_directories(Pong PUBLIC 
"${CMAKE_CURRENT_SOURCE_DIR}/include/" 
"${CMAKE_CURRENT_SOURCE_DIR}/include/asio-1.36.0/include"
)

target_link_libraries(Pong PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs/OpenAL32.lib")

# GLFW und GLM linken
target_link_libraries(Pong PRIVATE glfw)

# === Shader kopieren (res/shaders -> build/res/shaders) ===

set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders")
set(SHADER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/res/shaders")

file(GLOB SHADER_FILES "${SHADER_SOURCE_DIR}/*.shader")

foreach(SHADER_FILE ${SHADER_FILES})
    configure_file(${SHADER_FILE} ${SHADER_BUILD_DIR}/ COPYONLY)
endforeach()

target_compile_definitions(Pong PRIVATE SHADER_DIR="${SHADER_BUILD_DIR}")

# === Texturen kopieren (res/textures -> build/res/textures) ===

set(TEXTURE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/textures")
set(TEXTURE_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/res/textures")

file(GLOB TEXTURE_FILES "${TEXTURE_SOURCE_DIR}/*.*")

set(COPIED_TEXTURES "")
foreach(TEXTURE_FILE ${TEXTURE_FILES})
    get_filename_component(TEXTURE_NAME ${TEXTURE_FILE} NAME)
    set(TEXTURE_OUT "${TEXTURE_BUILD_DIR}/${TEXTURE_NAME}")

    add_custom_command(
        OUTPUT ${TEXTURE_OUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${TEXTURE_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TEXTURE_FILE} ${TEXTURE_OUT}
        DEPENDS ${TEXTURE_FILE}
        COMMENT "Kopiere Textur: ${TEXTURE_NAME}"
        VERBATIM
    )

    list(APPEND COPIED_TEXTURES ${TEXTURE_OUT})
endforeach()

# Custom target, das alle Texturen erzeugt
add_custom_target(copy_textures ALL DEPENDS ${COPIED_TEXTURES})

# Sorge dafür, dass dein Spiel erst nach den Texturen gebaut wird
add_dependencies(Pong copy_textures)

# Definiere TEXTURE_DIR für dein Programm
target_compile_definitions(Pong PRIVATE TEXTURE_DIR="${TEXTURE_BUILD_DIR}")

# === Sounds kopieren (res/sounds -> build/res/sounds) ===
set(SOUND_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/sounds")
set(SOUND_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/res/sounds")

file(GLOB SOUND_FILES "${SOUND_SOURCE_DIR}/*.*")

set(COPIED_SOUNDS "")
foreach(SOUND_FILE ${SOUND_FILES})
    get_filename_component(SOUND_NAME ${SOUND_FILE} NAME)
    set(SOUND_OUT "${SOUND_BUILD_DIR}/${SOUND_NAME}")

    add_custom_command(
        OUTPUT ${SOUND_OUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SOUND_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SOUND_FILE} ${SOUND_OUT}
        DEPENDS ${SOUND_FILE}
        COMMENT "Kopiere Sound: ${SOUND_NAME}"
        VERBATIM
    )

    list(APPEND COPIED_SOUNDS ${SOUND_OUT})
endforeach()

add_custom_target(copy_sounds ALL DEPENDS ${COPIED_SOUNDS})
add_dependencies(Pong copy_sounds)

target_compile_definitions(Pong PRIVATE SOUND_DIR="${SOUND_BUILD_DIR}")