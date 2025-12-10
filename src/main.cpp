#include <glad/glad.h>
#include <GLFW/glfw3.h>

// OpenAL
#include <AL/al.h>
#include <AL/alc.h>

// ImGui
#include "vendor/imgui/imgui.h"  
#include "vendor/imgui/imgui_impl_glfw.h"
#include "vendor/imgui/imgui_impl_opengl3.h"

//Assets
#include "AssetManager.h"

#include "Entitymanager.h"

#include "RenderSystem.h"

#include "Components.h"

#include "Game.h"
#include "GlobalSettings.h"

GLFWwindow* InitWindow()
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return nullptr;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = 1920;
    int height = 1080;

    GLFWwindow* window = glfwCreateWindow(width, height, "RPG-Looter", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        getchar();
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed\n";
        return NULL;
    }

    std::cout << "Using GL Version: " << glGetString(GL_VERSION) << std::endl;
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    return window;
}

Game game; // Global, damit der Callback darauf zugreifen kann

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    game.onWindowResize(window, width, height);
}

int main(void) {

    GLFWwindow* window = InitWindow();

    if (!window) return -1;

    glfwSwapInterval(1);

    bool audioAvailable = false;
    ALCdevice* device = alcOpenDevice(nullptr);
    if (!device) {
        std::cerr << "Failed to open OpenAL device, continuing without audio." << std::endl;
    } else {
        ALCcontext* context = alcCreateContext(device, nullptr);
        if (!context || !alcMakeContextCurrent(context)) {
            std::cerr << "Failed to create/make current OpenAL context, disabling audio." << std::endl;
            if (context) alcDestroyContext(context);
            alcCloseDevice(device);
        } else {
            std::cout << "OpenAL ready: " << alGetString(AL_VERSION) << std::endl;
            audioAvailable = true;
        }
    }
    game.setAudioAvailable(audioAvailable);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();


	game.setup(window);

    float lastTime = glfwGetTime();
    float deltaTime = 0.0f;
    
    // VSync tracking
    static bool lastVSyncState = true;

    // Resize Callback registrieren
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        !glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        
        // Apply VSync if it changed
        auto& settings = GlobalSettings::getInstance();
        if (settings.renderingSettings.vsyncEnabled != lastVSyncState) {
            game.applyVSync();
            lastVSyncState = settings.renderingSettings.vsyncEnabled;
        }
        
        // FPS limiting (when VSync is off)
        if (!settings.renderingSettings.vsyncEnabled && settings.renderingSettings.targetFPS > 0) {
            float targetFrameTime = 1.0f / settings.renderingSettings.targetFPS;
            if (deltaTime < targetFrameTime) {
                float sleepTime = targetFrameTime - deltaTime;
                // Busy wait for more precise timing
                while (glfwGetTime() - currentTime < targetFrameTime) {
                    // Spin
                }
                currentTime = glfwGetTime();
                deltaTime = currentTime - lastTime;
            }
        }
        
        lastTime = currentTime;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Handle F-key shortcuts for window toggles
        struct KeyBinding {
            int key;
            bool* windowFlag;
        };
        
        static constexpr size_t NUM_SHORTCUTS = 6;
        static bool keyStates[NUM_SHORTCUTS] = {false, false, false, false, false, false};
        
        KeyBinding bindings[] = {
            {GLFW_KEY_F1, &settings.windowVisibility.showPerformanceWindow},
            {GLFW_KEY_F2, &settings.windowVisibility.showConsoleWindow},
            {GLFW_KEY_F3, &settings.windowVisibility.showSceneHierarchy},
            {GLFW_KEY_F4, &settings.windowVisibility.showEntityEditor},
            {GLFW_KEY_F5, &settings.windowVisibility.showAssetManager},
            {GLFW_KEY_F6, &settings.windowVisibility.showModelEditor}
        };
        
        static_assert(sizeof(bindings)/sizeof(bindings[0]) == NUM_SHORTCUTS, 
                     "Number of key bindings must match NUM_SHORTCUTS");
        
        for (size_t i = 0; i < NUM_SHORTCUTS; ++i) {
            bool keyPressed = glfwGetKey(window, bindings[i].key) == GLFW_PRESS;
            if (keyPressed && !keyStates[i]) {
                *bindings[i].windowFlag = !(*bindings[i].windowFlag);
            }
            keyStates[i] = keyPressed;
        }
        
		game.update(deltaTime);

        // Note: Debug/Performance window is now handled by PerformanceWindow system
        // which is part of the game's systems
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
     
     // Swap buffers
        glfwSwapBuffers(window);
		glfwPollEvents();
        
    }
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        AssetManager::getInstance()->clear();

        // Cleanup OpenAL if it was initialized
    if (audioAvailable) {
        ALCcontext* current = alcGetCurrentContext();
        if (current) {
            ALCdevice* dev = alcGetContextsDevice(current);
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(current);
            if (dev) alcCloseDevice(dev);
        }
    }

        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;
    }











