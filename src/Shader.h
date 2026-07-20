#pragma once

#include <string>

class Shader {
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void use() const;
    void setMat4(const char* name, const float* values) const;
    void setVec3(const char* name, float x, float y, float z) const;
    void setInt(const char* name, int value) const;
    void setFloat(const char* name, float value) const;

private:
    unsigned int programId_ = 0;

    static std::string readFile(const char* path);
    static unsigned int compile(unsigned int type, const std::string& source);
};