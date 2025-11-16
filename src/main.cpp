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

    ALCdevice* device = alcOpenDevice(nullptr);
    if (!device) {
        std::cerr << "Failed to open OpenAL device" << std::endl;
        return -1;
    }

    ALCcontext* context = alcCreateContext(device, nullptr);
    alcMakeContextCurrent(context);
    std::cout << "OpenAL ready: " << alGetString(AL_VERSION) << std::endl;
    
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
    
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();


	game.setup(window);

    float lastTime = glfwGetTime();
    float deltaTime = 0.0f;

    // Resize Callback registrieren
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        !glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        
        float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
		game.update(deltaTime);

    
    
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
        ImGui::Begin("RPG-Looter - Debug");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::Text("DeltaTime: %.4f s", deltaTime);
        ImGui::Separator();
        ImGui::End();
        

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

        alcDestroyContext(context);
        alcCloseDevice(device);

        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;
    }






