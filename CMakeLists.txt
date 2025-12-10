cmake_minimum_required(VERSION 3.13)

project(RPG-Looter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW einbinden
add_subdirectory("extern/glfw-3.4")


# Executable definieren
add_executable(RPG-Looter

    "src/Debug.h"
    "src/Debug.cpp"

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

    "src/AudioSystem.h"
    "src/Audiosystem.cpp"
    "src/Component.h" 
    "src/Components.h" 
    "src/Entity.h" 
    "src/Entity.cpp" 
    "src/Entitymanager.cpp" 
    "src/Entitymanager.h" 
    "src/System.h" 
    "src/RenderSystem.h" 
    "src/RenderSystem.cpp" 
    "src/InputSystem.h" 
    "src/InputSystem.cpp"  
    "src/AssetManager.h" 
    "src/AssetManager.cpp" 
    "src/ECSSound.h" 
    "src/ECSSound.cpp" 

    "src/EditorSystem.h"
    "src/EditorSystem.cpp"
    "src/AssetManagerWindow.h"
    "src/AssetManagerWindow.cpp"
    "src/EntitySerializer.h"
    "src/EntitySerializer.cpp"
    "src/PerformanceWindow.h"
    "src/PerformanceWindow.cpp"
    "src/ConsoleWindow.h"
    "src/ConsoleWindow.cpp"
    "src/SceneHierarchyWindow.h"
    "src/SceneHierarchyWindow.cpp"
    "src/SettingsWindow.h"
    "src/SettingsWindow.cpp"
    "src/QuickActionsWindow.h"
    "src/QuickActionsWindow.cpp"
    "src/ModelEditorWindow.h"
    "src/ModelEditorWindow.cpp"
    "src/GlobalSettings.h"
    "src/CameraUtils.h"
    
    "src/main.cpp" "src/Game.h" "src/Game.cpp"  "src/JsonParser.h" "src/ModelSerializer.h" "src/ModelSerializer.cpp"

    # Collision system files
    "src/CollisionUtils.h"
    "src/CollisionSystem.h"
    "src/CollisionSystem.cpp"
    
    # Font system files
    "src/Font.h"
    "src/Font.cpp"
    
    # 3D Engine Components
    "src/Mesh3D.h"
    "src/Mesh3D.cpp"
    "src/PrimitiveGenerator.h"
    "src/PrimitiveGenerator.cpp"
    "src/OBJLoader.h"
    "src/OBJLoader.cpp"
    "src/Camera.h"
    "src/Camera.cpp"
    "src/FPSCamera.h"
    "src/FPSCamera.cpp"
    "src/OrbitalCamera.h"
    "src/OrbitalCamera.cpp"
    "src/CameraController.h"
    "src/CameraController.cpp"
    "src/PlayerControllerSystem.h"
    "src/PlayerControllerSystem.cpp"
    "src/RenderSystem3D.h"
    "src/RenderSystem3D.cpp"
    "src/DebugWindow.h"
    "src/DebugWindow.cpp")
    
    
# Includes
target_include_directories(RPG-Looter PUBLIC 
"${CMAKE_CURRENT_SOURCE_DIR}/include/" 
"${CMAKE_CURRENT_SOURCE_DIR}/include/asio-1.36.0/include"
)

# OpenAL linking - platform specific
if(WIN32)
    target_link_libraries(RPG-Looter PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs/OpenAL32.lib")
else()
    # On Linux/macOS, use system OpenAL
    find_package(OpenAL REQUIRED)
    target_link_libraries(RPG-Looter PRIVATE ${OPENAL_LIBRARY})
    target_include_directories(RPG-Looter PRIVATE ${OPENAL_INCLUDE_DIR})
endif()


# GLFW und GLM linken
target_link_libraries(RPG-Looter PRIVATE glfw)

# === Shader kopieren (res/shaders -> build/res/shaders) ===

set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/shaders")
set(SHADER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/res/shaders")

file(GLOB SHADER_FILES "${SHADER_SOURCE_DIR}/*.shader")

foreach(SHADER_FILE ${SHADER_FILES})
    configure_file(${SHADER_FILE} ${SHADER_BUILD_DIR}/ COPYONLY)
endforeach()

target_compile_definitions(RPG-Looter PRIVATE SHADER_DIR="${SHADER_BUILD_DIR}")

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

# Sorge dafuer, dass dein Spiel erst nach den Texturen gebaut wird
add_dependencies(RPG-Looter copy_textures)

# Definiere TEXTURE_DIR fuer dein Programm
target_compile_definitions(RPG-Looter PRIVATE TEXTURE_DIR="${TEXTURE_BUILD_DIR}")

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
add_dependencies(RPG-Looter copy_sounds)

target_compile_definitions(RPG-Looter PRIVATE SOUND_DIR="${SOUND_BUILD_DIR}")

# === Fonts kopieren (res/fonts -> build/res/fonts) ===
set(FONT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res/fonts")
set(FONT_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/res/fonts")

file(GLOB FONT_FILES "${FONT_SOURCE_DIR}/*.*")

set(COPIED_FONTS "")
foreach(FONT_FILE ${FONT_FILES})
    get_filename_component(FONT_NAME ${FONT_FILE} NAME)
    set(FONT_OUT "${FONT_BUILD_DIR}/${FONT_NAME}")

    add_custom_command(
        OUTPUT ${FONT_OUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${FONT_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FONT_FILE} ${FONT_OUT}
        DEPENDS ${FONT_FILE}
        COMMENT "Kopiere Font: ${FONT_NAME}"
        VERBATIM
    )

    list(APPEND COPIED_FONTS ${FONT_OUT})
endforeach()

add_custom_target(copy_fonts ALL DEPENDS ${COPIED_FONTS})
add_dependencies(RPG-Looter copy_fonts)

target_compile_definitions(RPG-Looter PRIVATE FONT_DIR="${FONT_BUILD_DIR}")

# === OpenAL DLLs kopieren ===
set(OPENAL_DLL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern/openal-soft")
set(OPENAL_DLLS
    "${OPENAL_DLL_DIR}/router/Win64/OpenAL32.dll"
    "${OPENAL_DLL_DIR}/bin/Win64/soft_oal.dll"
    "${OPENAL_DLL_DIR}/alsoft-config/libgcc_s_seh-1.dll"
    "${OPENAL_DLL_DIR}/alsoft-config/libstdc++-6.dll"
    "${OPENAL_DLL_DIR}/alsoft-config/libwinpthread-1.dll"
)

foreach(DLL_FILE ${OPENAL_DLLS})
    if(EXISTS ${DLL_FILE})
        get_filename_component(DLL_NAME ${DLL_FILE} NAME)
        add_custom_command(
            TARGET RPG-Looter POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${DLL_NAME}
            COMMENT "Kopiere DLL: ${DLL_NAME}"
        )
    else()
        message(WARNING "OpenAL DLL fehlt: ${DLL_FILE}")
    endif()
endforeach()