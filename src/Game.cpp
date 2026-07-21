#include "Game.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>

namespace {
constexpr float kPlayerSpeed = 350.0f;
constexpr float kBulletSpeed = 500.0f;
constexpr float kTurboBulletSpeedMultiplier = 1.6f;
constexpr float kEnemySpeed = 70.0f;
constexpr float kEnemyDropDistance = 24.0f;
constexpr float kEnemyBottomLimit = 80.0f;
constexpr float kBulletWidth = 6.0f;
constexpr float kBulletHeight = 16.0f;
constexpr float kFireCooldownNormal = 0.35f;
constexpr float kFireCooldownTurbo = 0.08f;
constexpr int kMaxBulletsNormal = 1;
constexpr int kMaxBulletsTurbo = 8;
}

Game::Game(int width, int height, const Color& playerColor)
    : windowWidth_(width),
      windowHeight_(height),
      player_{width * 0.5f - 30.0f, 30.0f, 60.0f, 20.0f},
      playerColor_(playerColor) {
    createEnemies();
}

void Game::tryFire(bool turbo) {
    const int maxBullets = turbo ? kMaxBulletsTurbo : kMaxBulletsNormal;
    if (static_cast<int>(bullets_.size()) >= maxBullets) {
        return;
    }

    Bullet bullet;
    bullet.turbo = turbo;
    bullet.rect = Rect{
        player_.x + player_.width * 0.5f - kBulletWidth * 0.5f,
        player_.y + player_.height,
        kBulletWidth,
        kBulletHeight
    };
    bullets_.push_back(bullet);
    fireCooldown_ = turbo ? kFireCooldownTurbo : kFireCooldownNormal;
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

    const bool spaceIsDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    const bool turboIsDown = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                             glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    if (spaceIsDown && fireCooldown_ <= 0.0f) {
        tryFire(turboIsDown);
    }
}

void Game::update(float deltaTime) {
    if (finished_) {
        return;
    }

    if (fireCooldown_ > 0.0f) {
        fireCooldown_ -= deltaTime;
    }

    for (auto bulletIt = bullets_.begin(); bulletIt != bullets_.end();) {
        const float speed = bulletIt->turbo ? kBulletSpeed * kTurboBulletSpeedMultiplier : kBulletSpeed;
        bulletIt->rect.y += speed * deltaTime;

        if (bulletIt->rect.y > static_cast<float>(windowHeight_)) {
            bulletIt = bullets_.erase(bulletIt);
            continue;
        }

        bool bulletHit = false;
        for (std::size_t index = 0; index < enemies_.size(); ++index) {
            if (intersects(bulletIt->rect, enemies_[index].rect)) {
                enemies_.erase(enemies_.begin() + static_cast<long>(index));
                bulletHit = true;
                break;
            }
        }

        if (bulletHit) {
            bulletIt = bullets_.erase(bulletIt);
        } else {
            ++bulletIt;
        }
    }

    bool reverseDirection = false;
    for (Enemy& enemy : enemies_) {
        enemy.rect.x += enemyDirection_ * kEnemySpeed * deltaTime;
        if (enemy.rect.x <= 0.0f || enemy.rect.x + enemy.rect.width >= static_cast<float>(windowWidth_)) {
            reverseDirection = true;
        }
    }

    if (reverseDirection) {
        enemyDirection_ *= -1.0f;
        for (Enemy& enemy : enemies_) {
            enemy.rect.y -= kEnemyDropDistance;
            enemy.rect.x = std::clamp(enemy.rect.x, 0.0f, static_cast<float>(windowWidth_) - enemy.rect.width);
        }
    }

    if (enemies_.empty()) {
        finishGame(true);
        return;
    }

    for (const Enemy& enemy : enemies_) {
        if (enemy.rect.y <= kEnemyBottomLimit) {
            finishGame(false);
            return;
        }
    }
}

void Game::render(const Shader& shader, unsigned int quadVao) const {
    drawRect(shader, quadVao, player_, playerColor_.red, playerColor_.green, playerColor_.blue);

    for (const Bullet& bullet : bullets_) {
        if (bullet.turbo) {
            drawRect(shader, quadVao, bullet.rect, 1.0f, 0.75f, 0.2f);
        } else {
            drawRect(shader, quadVao, bullet.rect, 1.0f, 1.0f, 1.0f);
        }
    }

    for (const Enemy& enemy : enemies_) {
        drawRect(shader, quadVao, enemy.rect, enemy.color.red, enemy.color.green, enemy.color.blue);
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

    // Distinct colors per invasion row (top → bottom).
    constexpr Color rowColors[rows] = {
        {0.95f, 0.35f, 0.85f},  // magenta
        {0.35f, 0.75f, 1.0f},   // cyan
        {1.0f, 0.75f, 0.2f},    // gold
        {0.9f, 0.25f, 0.25f},   // red
    };

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            enemies_.push_back(Enemy{
                Rect{
                    startX + column * (enemyWidth + spacingX),
                    startY + row * (enemyHeight + spacingY),
                    enemyWidth,
                    enemyHeight
                },
                rowColors[row]
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
