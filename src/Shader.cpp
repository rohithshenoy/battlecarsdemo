#include "Shader.h"

#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    const std::string vertexSource = readFile(vertexPath);
    const std::string fragmentSource = readFile(fragmentPath);

    const unsigned int vertexShader = compile(GL_VERTEX_SHADER, vertexSource);
    const unsigned int fragmentShader = compile(GL_FRAGMENT_SHADER, fragmentSource);

    programId_ = glCreateProgram();
    glAttachShader(programId_, vertexShader);
    glAttachShader(programId_, fragmentShader);
    glLinkProgram(programId_);

    int success = 0;
    glGetProgramiv(programId_, GL_LINK_STATUS, &success);
    if (success == 0) {
        char infoLog[512] = {};
        glGetProgramInfoLog(programId_, sizeof(infoLog), nullptr, infoLog);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(programId_);
        throw std::runtime_error(std::string("Failed to link shader program: ") + infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    if (programId_ != 0) {
        glDeleteProgram(programId_);
    }
}

void Shader::use() const {
    glUseProgram(programId_);
}

void Shader::setMat4(const char* name, const float* values) const {
    const int location = glGetUniformLocation(programId_, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, values);
}

void Shader::setVec3(const char* name, float x, float y, float z) const {
    const int location = glGetUniformLocation(programId_, name);
    glUniform3f(location, x, y, z);
}

std::string Shader::readFile(const char* path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error(std::string("Failed to open shader file: ") + path);
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

unsigned int Shader::compile(unsigned int type, const std::string& source) {
    const unsigned int shaderId = glCreateShader(type);
    const char* sourcePtr = source.c_str();
    glShaderSource(shaderId, 1, &sourcePtr, nullptr);
    glCompileShader(shaderId);

    int success = 0;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (success == 0) {
        char infoLog[512] = {};
        glGetShaderInfoLog(shaderId, sizeof(infoLog), nullptr, infoLog);
        glDeleteShader(shaderId);
        throw std::runtime_error(std::string("Failed to compile shader: ") + infoLog);
    }

    return shaderId;
}