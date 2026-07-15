#include "Game.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>

namespace {
constexpr float kPlayerSpeed = 350.0f;
constexpr float kBulletSpeed = 500.0f;
constexpr float kEnemySpeed = 70.0f;
constexpr float kEnemyDropDistance = 24.0f;
constexpr float kEnemyBottomLimit = 80.0f;
}

Game::Game(int width, int height, const Color& playerColor)
    : windowWidth_(width),
      windowHeight_(height),
      player_{width * 0.5f - 30.0f, 30.0f, 60.0f, 20.0f},
      playerColor_(playerColor),
      bullet_{0.0f, 0.0f, 6.0f, 16.0f} {
    createEnemies();
}

void Game::processInput(GLFWwindow* window, float deltaTime) {
    if (finished_) {
        return;
    }

    float horizontal = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        horizontal -= 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        horizontal += 1.0f;
    }

    player_.x += horizontal * kPlayerSpeed * deltaTime;
    player_.x = std::clamp(player_.x, 0.0f, static_cast<float>(windowWidth_) - player_.width);

    static bool spaceWasDown = false;
    const bool spaceIsDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    if (spaceIsDown && !spaceWasDown && !bulletActive_) {
        bulletActive_ = true;
        bullet_.x = player_.x + player_.width * 0.5f - bullet_.width * 0.5f;
        bullet_.y = player_.y + player_.height;
    }
    spaceWasDown = spaceIsDown;
}

void Game::update(float deltaTime) {
    if (finished_) {
        return;
    }

    if (bulletActive_) {
        bullet_.y += kBulletSpeed * deltaTime;
        if (bullet_.y > static_cast<float>(windowHeight_)) {
            bulletActive_ = false;
        }
    }

    bool reverseDirection = false;
    for (Rect& enemy : enemies_) {
        enemy.x += enemyDirection_ * kEnemySpeed * deltaTime;
        if (enemy.x <= 0.0f || enemy.x + enemy.width >= static_cast<float>(windowWidth_)) {
            reverseDirection = true;
        }
    }

    if (reverseDirection) {
        enemyDirection_ *= -1.0f;
        for (Rect& enemy : enemies_) {
            enemy.y -= kEnemyDropDistance;
            enemy.x = std::clamp(enemy.x, 0.0f, static_cast<float>(windowWidth_) - enemy.width);
        }
    }

    if (bulletActive_) {
        for (std::size_t index = 0; index < enemies_.size(); ++index) {
            if (intersects(bullet_, enemies_[index])) {
                bulletActive_ = false;
                enemies_.erase(enemies_.begin() + static_cast<long>(index));
                break;
            }
        }
    }

    if (enemies_.empty()) {
        finishGame(true);
        return;
    }

    for (const Rect& enemy : enemies_) {
        if (enemy.y <= kEnemyBottomLimit) {
            finishGame(false);
            return;
        }
    }
}

void Game::render(const Shader& shader, unsigned int quadVao) const {
    drawRect(shader, quadVao, player_, playerColor_.red, playerColor_.green, playerColor_.blue);

    if (bulletActive_) {
        drawRect(shader, quadVao, bullet_, 1.0f, 1.0f, 1.0f);
    }

    for (const Rect& enemy : enemies_) {
        drawRect(shader, quadVao, enemy, 0.9f, 0.2f, 0.2f);
    }
}

bool Game::hasEnded() const {
    return finished_;
}

bool Game::intersects(const Rect& a, const Rect& b) {
    return a.x < b.x + b.width &&
           a.x + a.width > b.x &&
           a.y < b.y + b.height &&
           a.y + a.height > b.y;
}

void Game::buildModelMatrix(const Rect& rect, float* model) {
    model[0] = rect.width;
    model[1] = 0.0f;
    model[2] = 0.0f;
    model[3] = 0.0f;

    model[4] = 0.0f;
    model[5] = rect.height;
    model[6] = 0.0f;
    model[7] = 0.0f;

    model[8] = 0.0f;
    model[9] = 0.0f;
    model[10] = 1.0f;
    model[11] = 0.0f;

    model[12] = rect.x;
    model[13] = rect.y;
    model[14] = 0.0f;
    model[15] = 1.0f;
}

void Game::drawRect(const Shader& shader, unsigned int quadVao, const Rect& rect, float red, float green, float blue) const {
    float model[16] = {};
    buildModelMatrix(rect, model);

    shader.setMat4("model", model);
    shader.setVec3("spriteColor", red, green, blue);

    glBindVertexArray(quadVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Game::createEnemies() {
    enemies_.clear();

    constexpr int rows = 4;
    constexpr int columns = 8;
    constexpr float enemyWidth = 48.0f;
    constexpr float enemyHeight = 24.0f;
    constexpr float spacingX = 18.0f;
    constexpr float spacingY = 18.0f;
    constexpr float startX = 90.0f;
    constexpr float startY = 480.0f;

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            enemies_.push_back(Rect{
                startX + column * (enemyWidth + spacingX),
                startY + row * (enemyHeight + spacingY),
                enemyWidth,
                enemyHeight
            });
        }
    }
}

void Game::finishGame(bool playerWon) {
    finished_ = true;
    playerWon_ = playerWon;

    if (playerWon_) {
        std::cout << "You win!" << std::endl;
    } else {
        std::cout << "You lose!" << std::endl;
    }
}