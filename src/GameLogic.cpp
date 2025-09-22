#include "GameLogic.h"
#include "vendor/glm/gtc/constants.hpp"
#include <random>
#include <algorithm>

// ================= Hilfsfunktionen =================
float randomFloat(float min, float max) {
    if (min > max) std::swap(min, max);
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}

template <typename T>
T clamp(T value, T minVal, T maxVal) {
    return (value < minVal) ? minVal : (value > maxVal) ? maxVal : value;
}

// ================= Paddle =================
Paddle::Paddle(const glm::vec2& position, const glm::vec2& size, int player1or2)
    : m_position(position), m_size(size), m_PlayerNumber(player1or2), speed(600.0f) {
    m_indices = { 0, 1, 2, 2, 3, 0 };
}

void Paddle::set_position(const glm::vec2& newPos) { m_position = newPos; }
void Paddle::set_velocity(const glm::vec2& newvel) { m_velocity = newvel; }
void Paddle::set_size(const glm::vec2& newsize) { m_size = newsize; }
void Paddle::set_speed(float newSpeed) { speed = newSpeed; }
void Paddle::set_indices(const std::vector<unsigned int>& indices) { m_indices = indices; }

glm::vec2 Paddle::get_position() const { return m_position; }
glm::vec2 Paddle::get_velocity() const { return m_velocity; }
glm::vec2 Paddle::get_size() const { return m_size; }
int Paddle::get_speed() const { return speed; }
const std::vector<unsigned int>& Paddle::GetIndices() const { return m_indices; }

std::vector<Vertex> Paddle::GetVertices() const {
    float halfWidth = m_size.x * 0.5f;
    float halfHeight = m_size.y * 0.5f;
    float z = 0.0f;

    glm::vec2 uv0(0.0f, 0.0f);
    glm::vec2 uv1(1.0f, 0.0f);
    glm::vec2 uv2(1.0f, 1.0f);
    glm::vec2 uv3(0.0f, 1.0f);

    std::vector<Vertex> vertices;
    vertices.reserve(4); // optional, 4 Vertices

    vertices.push_back(Vertex{ {-halfWidth, -halfHeight, z}, uv0 });
    vertices.push_back(Vertex{ { halfWidth, -halfHeight, z}, uv1 });
    vertices.push_back(Vertex{ { halfWidth,  halfHeight, z}, uv2 });
    vertices.push_back(Vertex{ {-halfWidth,  halfHeight, z}, uv3 });

    return vertices;
}

void Paddle::Update(float deltaTime, GLFWwindow* window) {
    m_velocity = { 0.0f, 0.0f };

    if (m_PlayerNumber == 1) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) m_velocity.y = speed;
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) m_velocity.y = -speed;
    }
    else if (m_PlayerNumber == 2) {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) m_velocity.y = speed;
        else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) m_velocity.y = -speed;
    }

    m_position += m_velocity * deltaTime;

    float top = 1080.0f - m_size.y / 2.0f;
    float bottom = m_size.y / 2.0f;

    if (m_position.y > top) m_position.y = top;
    if (m_position.y < bottom) m_position.y = bottom;
}

// ================= Score =================
Score::Score() : player1Score(0), player2Score(0) {}

void Score::setscore(int player, int score) {
    if (player == 1) player1Score = score;
    else if (player == 2) player2Score = score;
}

int Score::getPlayer1Score() const { return player1Score; }
int Score::getPlayer2Score() const { return player2Score; }
void Score::resetScores() { player1Score = 0; player2Score = 0; }

// ================= Ball =================
Ball::Ball(const glm::vec2& position, float radius, int segments)
    : m_position(position),
    m_radius(radius),
    m_originalRadius(radius),
    m_segments(segments),
    m_speed(600.0f),
    m_isAnimating(false),
    m_animationEnabled(true),
    m_animationTimer(0.0f),
    m_animationStep(0),
    m_totalAnimationSteps(5),
    m_animationDuration(0.5f),
    m_animationStartRadius(0.0f),
    top(1080.0f - radius),
    bottom(radius),
    xleftmax(radius),
    xrightmax(1920.0f - radius) {
}

void Ball::set_position(const glm::vec2& newPos) { m_position = newPos; }
void Ball::set_velocity(const glm::vec2& newVel) { m_velocity = newVel; }
void Ball::set_radius(float radius) { m_radius = radius; }
void Ball::set_speed(float speed) { m_speed = speed; }
void Ball::set_animation_steps(int steps) { m_totalAnimationSteps = steps; }
void Ball::set_animation_duration(float duration) { m_animationDuration = duration; }
void Ball::SetAnimationEnabled(bool enabled) { m_animationEnabled = enabled; }

void Ball::reset(const glm::vec2& position, const glm::vec2& velocity) {
    m_position = position;
    m_velocity = velocity;
}

glm::vec2 Ball::get_position() const { return m_position; }
glm::vec2 Ball::get_velocity() const { return m_velocity; }
float Ball::get_radius() const { return m_radius; }
float Ball::get_speed() const { return m_speed; }
float Ball::getOriginalRadius() const { return m_originalRadius; }

std::vector<glm::vec3> Ball::GetVertices() const {
    std::vector<glm::vec3> vertices;
    float z = 0.0f;

    // Mittelpunkt (Kreiszentrum) als Vertex für TRIANGLE_FAN
    vertices.emplace_back(0.0f, 0.0f, z);

    for (int i = 0; i <= m_segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * i / m_segments;
        float x = m_radius * cos(angle);
        float y = m_radius * sin(angle);
        vertices.emplace_back(x, y, z);
    }

    return vertices;
}

std::vector<glm::vec2> Ball::GetTexCoords() const {
    std::vector<glm::vec2> texCoords;

    // Zentrum = Mitte der Textur
    texCoords.emplace_back(0.5f, 0.5f);

    for (int i = 0; i <= m_segments; ++i) {
        float angle = 2.0f * glm::pi<float>() * i / m_segments;
        float u = 0.5f + 0.5f * cos(angle); // x → [0,1]
        float v = 0.5f + 0.5f * sin(angle); // y → [0,1]
        texCoords.emplace_back(u, v);
    }

    return texCoords;
}

std::vector<unsigned int> Ball::GetIndices() const {
    std::vector<unsigned int> indices;
    for (int i = 1; i <= m_segments; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    indices.push_back(0);
    indices.push_back(m_segments);
    indices.push_back(1);
    return indices;
}

void Ball::StartScaleAnimation() {
    if (!m_animationEnabled) return;
    m_isAnimating = true;
    m_animationTimer = 0.0f;
    m_animationStep = 0;
    m_animationStartRadius = m_originalRadius * 0.25f;
    m_radius = m_animationStartRadius;
}

void Ball::Update(float deltaTime, Score& score) {
    if (m_isAnimating) {
        float stepTime = m_animationDuration / static_cast<float>(m_totalAnimationSteps);
        m_animationTimer += deltaTime;

        if (m_animationTimer >= stepTime) {
            m_animationStep++;
            m_animationTimer = 0.0f;
            float t = static_cast<float>(m_animationStep) / m_totalAnimationSteps;
            m_radius = m_animationStartRadius + (m_originalRadius - m_animationStartRadius) * t;

            if (m_animationStep >= m_totalAnimationSteps) {
                m_isAnimating = false;
                m_radius = m_originalRadius;
            }
        }
        return;
    }

    m_position += m_velocity * m_speed * deltaTime;

    if (m_position.y > top) { m_position.y = top; m_velocity.y *= -1; }
    if (m_position.y < bottom) { m_position.y = bottom; m_velocity.y *= -1; }

    float random_direction = randomFloat(0.1f, 0.99f);
    float random_directionn = randomFloat(-0.1f, -0.99f);

    if (m_position.x < xleftmax) {
        score.setscore(2, score.getPlayer2Score() + 1);
        m_position = { 960.0f, 540.0f };
        m_velocity = glm::normalize(glm::vec2(1.0f, random_direction));
        StartScaleAnimation();
    }
    if (m_position.x > xrightmax) {
        score.setscore(1, score.getPlayer1Score() + 1);
        m_position = { 960.0f, 540.0f };
        m_velocity = glm::normalize(glm::vec2(-1.0f, random_directionn));
        StartScaleAnimation();
    }
}

bool Ball::CheckCollision(const Paddle& paddle) const {
    glm::vec2 paddlePos = paddle.get_position();
    glm::vec2 halfSize = paddle.get_size() * 0.5f;

    float closestX = std::max(paddlePos.x - halfSize.x, std::min(m_position.x, paddlePos.x + halfSize.x));
    float closestY = std::max(paddlePos.y - halfSize.y, std::min(m_position.y, paddlePos.y + halfSize.y));

    float distanceX = m_position.x - closestX;
    float distanceY = m_position.y - closestY;
    float distanceSquared = distanceX * distanceX + distanceY * distanceY;

    return distanceSquared <= (m_radius * m_radius);
}

// ================= DeltaTime =================
DeltaTime::DeltaTime()
    : m_LastTime(static_cast<float>(glfwGetTime())), m_DeltaTime(0.0f) {
}

void DeltaTime::Update() {
    float currentTime = static_cast<float>(glfwGetTime());
    m_DeltaTime = currentTime - m_LastTime;
    m_LastTime = currentTime;
}

float DeltaTime::get_deltatime() const { return m_DeltaTime; }
