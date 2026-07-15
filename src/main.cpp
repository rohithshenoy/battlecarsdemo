#include "Game.h"
#include "Shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;
constexpr Color kDefaultPlayerColor{0.2f, 0.9f, 0.3f};
constexpr char kInvalidSpaceshipColorMessage[] =
    "Invalid spaceship color. Use --spaceship-color=r,g,b with values between 0.0 and 1.0.";
constexpr char kSpaceshipColorPrefix[] = "--spaceship-color=";
constexpr auto kSpaceshipColorPrefixLength = sizeof(kSpaceshipColorPrefix) - 1;

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void makeOrthographic(float left, float right, float bottom, float top, float* matrix) {
    matrix[0] = 2.0f / (right - left);
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = 2.0f / (top - bottom);
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = -1.0f;
    matrix[11] = 0.0f;

    matrix[12] = -(right + left) / (right - left);
    matrix[13] = -(top + bottom) / (top - bottom);
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}

bool parseSpaceshipColorValue(const std::string& value, Color* color, std::string* errorMessage) {
    std::istringstream input(value);
    Color parsedColor{0.0f, 0.0f, 0.0f};
    char separator = '\0';

    if (!(input >> parsedColor.red >> separator) || separator != ',' ||
        !(input >> parsedColor.green >> separator) || separator != ',' ||
        !(input >> parsedColor.blue)) {
        *errorMessage = kInvalidSpaceshipColorMessage;
        return false;
    }

    input >> std::ws;
    if (!input.eof()) {
        *errorMessage = kInvalidSpaceshipColorMessage;
        return false;
    }

    if (parsedColor.red < 0.0f || parsedColor.red > 1.0f ||
        parsedColor.green < 0.0f || parsedColor.green > 1.0f ||
        parsedColor.blue < 0.0f || parsedColor.blue > 1.0f) {
        *errorMessage = "Invalid spaceship color. Each color component must be between 0.0 and 1.0.";
        return false;
    }

    *color = parsedColor;
    return true;
}

bool parseSpaceshipColor(int argc, char** argv, Color* playerColor, std::string* errorMessage) {
    *playerColor = kDefaultPlayerColor;

    for (int argIndex = 1; argIndex < argc; ++argIndex) {
        const std::string argument = argv[argIndex];
        if (argument == "--spaceship-color") {
            if (argIndex + 1 >= argc) {
                *errorMessage = "Missing value for --spaceship-color.";
                return false;
            }
            const std::string value = argv[++argIndex];
            return parseSpaceshipColorValue(value, playerColor, errorMessage);
        }

        if (argument.compare(0, kSpaceshipColorPrefixLength, kSpaceshipColorPrefix) == 0) {
            return parseSpaceshipColorValue(argument.substr(kSpaceshipColorPrefixLength), playerColor, errorMessage);
        }
    }

    return true;
}
}

int main(int argc, char** argv) {
    Color playerColor = kDefaultPlayerColor;
    std::string playerColorError;
    if (!parseSpaceshipColor(argc, argv, &playerColor, &playerColorError)) {
        std::cerr << playerColorError << std::endl;
        return 1;
    }

    if (glfwInit() == GLFW_FALSE) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _WIN32
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, "Space Invaders", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    const float quadVertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    unsigned int vao = 0;
    unsigned int vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    try {
        const std::string shaderDir = SHADER_DIR;
        Shader shader((shaderDir + "/vertex.glsl").c_str(), (shaderDir + "/fragment.glsl").c_str());

        float projection[16] = {};
        makeOrthographic(0.0f, static_cast<float>(kWindowWidth), 0.0f, static_cast<float>(kWindowHeight), projection);

        shader.use();
        shader.setMat4("projection", projection);

        Game game(kWindowWidth, kWindowHeight, playerColor);

        auto previousTime = std::chrono::steady_clock::now();
        while (!glfwWindowShouldClose(window)) {
            const auto currentTime = std::chrono::steady_clock::now();
            const std::chrono::duration<float> elapsed = currentTime - previousTime;
            previousTime = currentTime;
            const float deltaTime = elapsed.count();

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || game.hasEnded()) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }

            game.processInput(window, deltaTime);
            game.update(deltaTime);

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            shader.use();
            game.render(shader, vao);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}