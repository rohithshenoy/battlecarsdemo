#pragma once

#include "Shader.h"

#include <vector>

struct Rect {
    float x;
    float y;
    float width;
    float height;
};

class GLFWwindow;

class Game {
public:
    Game(int width, int height);

    void processInput(GLFWwindow* window, float deltaTime);
    void update(float deltaTime);
    void render(const Shader& shader, unsigned int quadVao) const;

    bool hasEnded() const;

private:
    int windowWidth_;
    int windowHeight_;
    Rect player_;
    Rect bullet_;
    bool bulletActive_ = false;
    std::vector<Rect> enemies_;
    float enemyDirection_ = 1.0f;
    bool finished_ = false;
    bool playerWon_ = false;

    static bool intersects(const Rect& a, const Rect& b);
    static void buildModelMatrix(const Rect& rect, float* model);
    void drawRect(const Shader& shader, unsigned int quadVao, const Rect& rect, float red, float green, float blue) const;
    void createEnemies();
    void finishGame(bool playerWon);
};