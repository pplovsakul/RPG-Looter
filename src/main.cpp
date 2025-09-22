// Include standard headers
    #include <iostream>
    #include <vector>
    #include <string>
    #include <unordered_map>
    #include <string>
    #include <fstream>
    #include <sstream>
    #include <thread>
    #include <chrono>
    #include <cstdlib>
    #include <array>
    #include <ctime>
    #include <thread>
    #include <atomic>
    #include <random>
    #include <algorithm>
// Include ASIO
    #define ASIO_STANDALONE
    #include <asio.hpp>

// Include GLAD
    #include <glad/glad.h>

// Include GLFW
    #include <GLFW/glfw3.h>

// OpenAL
   #include <AL/al.h>
   #include <AL/alc.h>
   #include "Audiomanager.h"

//Include GLM
    #include "vendor/glm/glm.hpp"
    #include "vendor/glm/gtc/matrix_transform.hpp"

//Include ImGui
    #include "vendor/imgui/imgui.h"  
    #include "vendor/imgui/imgui_impl_glfw.h"
    #include "vendor/imgui/imgui_impl_opengl3.h"

// Include project files
    #include "Debug.h"
    #include "GameLogic.h"
    #include "Renderer.h"
    #include "IndexBuffer.h"
    #include "VertexArray.h"
    #include "VertexBuffer.h"
    #include "VertexBufferLayout.h"
    #include "Shader.h"
    #include "Texture.h"
    #include "Menu.h"
    #include "Rendermanager.h"

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
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Pong", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
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

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    return window;
}

    int main(void)
    {
        ALCdevice* device = alcOpenDevice(nullptr);
		if (!device) {
			std::cerr << "Failed to open OpenAL device" << std::endl;
			return -1;
		}

        ALCcontext* context = alcCreateContext(device, nullptr);
        alcMakeContextCurrent(context);
        std::cout << "OpenAL ready: " << alGetString(AL_VERSION) << std::endl;

        Sound hitSound;
        std::string err;
        if (!hitSound.LoadFromFile("res/sounds/explosion.wav", &err)) {
            std::cerr << "Sound load error: " << err << "\n";
        }
        else {
            hitSound.SetVolume(0.05f); // etwas leiser
			hitSound.SetPitch(5.0f);  // etwas schneller
        }

        GLFWwindow* window = InitWindow();
        if (!window)
            return -1;
        glfwSwapInterval(1);  // VSync an: Frame wird an Monitor-Refresh synchronisiert

        Renderer renderer;
        DeltaTime deltaTimeCalculator;
        Score score;

        Paddle paddle1(glm::vec2(50.0f, 540.0f), glm::vec2(20.0f, 150.0f), 1);  // Spieler 1 (links)
        Paddle paddle2(glm::vec2(1870.0f, 540.0f), glm::vec2(20.0f, 150.0f), 2); // Spieler 2 (rechts)
        
        Shader shader("res/shaders");
        shader.Bind();
        glm::mat4 proj = glm::ortho(0.0f, 1920.0f, 0.0f , 1080.0f, -1.0f, 10.0f);

        Texture playbutton_texture("res/textures/Lokal_spielen.png");
		Texture ball_texture("res/textures/pinkes_paddle.png");
		Texture rotesPaddle_texture("res/textures/rotes_paddle.png");
		Texture grünesPaddle_texture("res/textures/grünes_paddle.png");
        shader.SetUniform1i("u_Texture", 0);
       
		Ball ball(glm::vec2(960.0f, 540.0f), 20.0f, 30);
        ball.set_velocity(glm::normalize(glm::vec2(-1.0f, 0.3f)));

		RenderManager renderManager(renderer, shader, proj);

        Button play_button(960.0f, 540.0f, 300.0f, 100.0f);
		play_button.setOnClick([&]() {
			std::cout << "Play button clicked!" << std::endl;
			});

        std::vector<glm::vec3> positions = ball.GetVertices();
        std::vector<glm::vec2> texCoords = ball.GetTexCoords();

        // Kombiniere in ein Array aus Position + TexCoord
        struct Vertex {
            glm::vec3 pos;
            glm::vec2 uv;
        };

        std::vector<Vertex> vertices;
        vertices.reserve(positions.size());
        for (size_t i = 0; i < positions.size(); i++) {
            vertices.push_back({ positions[i], texCoords[i] });
        }

        renderManager.AddObject("ball", std::make_unique<RenderObject>(
            vertices.data(), vertices.size() * sizeof(Vertex),
            ball.GetIndices(), ball.get_position(),
            std::make_unique<Texture>(ball_texture)
        ));

        renderManager.AddObject("paddle1", std::make_unique<RenderObject>(
            paddle1.GetVertices().data(), paddle1.GetVertices().size() * sizeof(Vertex),
            paddle1.GetIndices(), paddle1.get_position(),
            std::make_unique<Texture>(rotesPaddle_texture)
        ));
        renderManager.AddObject("paddle2", std::make_unique<RenderObject>(
            paddle2.GetVertices().data(), paddle2.GetVertices().size() * sizeof(Vertex),
            paddle2.GetIndices(), paddle2.get_position(),
            std::make_unique<Texture>(grünesPaddle_texture)
        ));

		renderManager.AddObject("play_button", std::make_unique<RenderObject>(
			play_button.GetVertices().data(), play_button.GetVertices().size() * sizeof(Vertex),
			play_button.GetIndices(), play_button.get_position(),
			std::make_unique<Texture>(playbutton_texture)
		));
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGui::StyleColorsDark();
    
		int spiel_geschwindigkeit = 600; 
     
		do 
        {
            renderer.Clear();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            

            // DeltaTime aktualisieren
            deltaTimeCalculator.Update();

			// Ball aktualisieren
            ball.Update(deltaTimeCalculator.get_deltatime(), score);
            play_button.isClicked(window);

			// Paddle aktualisieren
			paddle1.Update(deltaTimeCalculator.get_deltatime(), window);
			paddle2.Update(deltaTimeCalculator.get_deltatime(), window);

            //Kollision prüfen und reagieren
            if (ball.CheckCollision(paddle1)) {
                glm::vec2 vel = ball.get_velocity();
                vel.x *= -1.0f;
                ball.set_velocity(vel);

                // Ball zurücksetzen an den Rand des Paddles, um Durchfliegen zu verhindern
                glm::vec2 paddlePos = paddle1.get_position();
                glm::vec2 halfSize = paddle1.get_size() * 0.5f;
                ball.set_position(glm::vec2(paddlePos.x + halfSize.x + ball.get_radius(), ball.get_position().y));
				hitSound.Play();
            }

            if (ball.CheckCollision(paddle2)) {
                glm::vec2 vel = ball.get_velocity();
                vel.x *= -1.0f;
                ball.set_velocity(vel);

                glm::vec2 paddlePos = paddle2.get_position();
                glm::vec2 halfSize = paddle2.get_size() * 0.5f;
                ball.set_position(glm::vec2(paddlePos.x - halfSize.x - ball.get_radius(), ball.get_position().y));
				hitSound.Play();
            }
            
            //zeichnen
			renderManager.UpdateObject("ball", ball.get_position());
			renderManager.UpdateObject("paddle1", paddle1.get_position());
			renderManager.UpdateObject("paddle2", paddle2.get_position());
			renderManager.DrawAll();

			// ImGui Fenster
			{
                ImGui::SetNextWindowPos(ImVec2(320, 10), ImGuiCond_Always);  // Zweites Fenster daneben
                ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Always);
				ImGui::Begin("Pong Game Controls");
				ImGui::Text("Use W/S for Player 1 (Left Paddle)");
				ImGui::Text("Use Up/Down Arrow for Player 2 (Right Paddle)");
				ImGui::Separator();
				ImGui::Text("Paddle 1 Position: (%.1f, %.1f)", paddle1.get_position().x, paddle1.get_position().y);
				ImGui::Text("Paddle 2 Position: (%.1f, %.1f)", paddle2.get_position().x, paddle2.get_position().y);
                ImGui::Separator();
				ImGui::Text("Ball Position: (%.1f, %.1f)", ball.get_position().x, ball.get_position().y);
                ImGui::Separator();
                if (ImGui::SliderInt("Spiel Geschwindigkeit:", &spiel_geschwindigkeit, 600, 10000)) {
                    paddle1.set_speed((float)spiel_geschwindigkeit);
                    paddle2.set_speed((float)spiel_geschwindigkeit);
                    ball.set_speed((float)spiel_geschwindigkeit);
                }
                ImGui::Separator();
                ImGui::Text("Spieler 1: %d  -  Spieler 2: %d", score.getPlayer1Score(), score.getPlayer2Score());
                ImGui::Separator();
                ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
                ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
				ImGui::End();
			}

            // ImGui rendern
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


            glfwSwapBuffers(window);
            glfwPollEvents(); 
		
		}  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window));

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        alcDestroyContext(context);
        alcCloseDevice(device);

        // Close OpenGL window and terminate GLFW
        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;


    };