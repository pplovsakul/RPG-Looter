#pragma once

#include <vector>
#include "vendor/glm/glm.hpp"
#include <GLFW/glfw3.h>

struct Vertex {
	glm::vec3 position;
	glm::vec2 texCoord;
};

// ================= Hilfsfunktionen =================
float randomFloat(float min, float max);

template <typename T>
T clamp(T value, T minVal, T maxVal);

// ================= Paddle =================
class Paddle {
private:
    glm::vec2 m_position;
    glm::vec2 m_velocity;
    glm::vec2 m_size;
    int m_PlayerNumber;
    std::vector<unsigned int> m_indices;
    float speed;

public:
    Paddle(const glm::vec2& position, const glm::vec2& size, int player1or2);

    // Setter
    void set_position(const glm::vec2& newPos);
    void set_velocity(const glm::vec2& newvel);
    void set_size(const glm::vec2& newsize);
    void set_speed(float newSpeed);
    void set_indices(const std::vector<unsigned int>& indices);

    // Getter
    glm::vec2 get_position() const;
    glm::vec2 get_velocity() const;
    glm::vec2 get_size() const;
    int get_speed() const;
    int get_player_id() const { return m_PlayerNumber; }
    const std::vector<unsigned int>& GetIndices() const;
    std::vector<Vertex> GetVertices() const;

    void Update(float deltaTime, GLFWwindow* window);
};

// ================= Score =================
class Score {
private:
    int player1Score;
    int player2Score;

public:
    Score();
    void setscore(int player, int score);
    int getPlayer1Score() const;
    int getPlayer2Score() const;
    void resetScores();
};

// ================= Ball =================
class Ball {
private:
    glm::vec2 m_position;
    glm::vec2 m_velocity;
    float m_radius;
    float m_originalRadius;
    int m_segments;
    float m_speed;

    // Animation
    bool m_isAnimating;
    bool m_animationEnabled;
    float m_animationTimer;
    int m_animationStep;
    int m_totalAnimationSteps;
    float m_animationDuration;
    float m_animationStartRadius;

    // Spielfeldbegrenzungen
    float top;
    float bottom;
    float xleftmax;
    float xrightmax;

public:
    Ball(const glm::vec2& position, float radius, int segments);

    // Setter
    void set_position(const glm::vec2& newPos);
    void set_velocity(const glm::vec2& newVel);
    void set_radius(float radius);
    void set_speed(float speed);
    void set_animation_steps(int steps);
    void set_animation_duration(float duration);
    void SetAnimationEnabled(bool enabled);

    void reset(const glm::vec2& position, const glm::vec2& velocity);

    // Getter
    glm::vec2 get_position() const;
    glm::vec2 get_velocity() const;
    float get_radius() const;
    float get_speed() const;
    float getOriginalRadius() const;

    std::vector<glm::vec3> GetVertices() const;
    std::vector<glm::vec2> Ball::GetTexCoords() const;
    std::vector<unsigned int> GetIndices() const;

    void StartScaleAnimation();
    void Update(float deltaTime, Score& score);
    bool CheckCollision(const Paddle& paddle) const;
};

// ================= DeltaTime =================
class DeltaTime {
public:
    DeltaTime();

    void Update();
    float get_deltatime() const;

private:
    float m_LastTime;
    float m_DeltaTime;
};

