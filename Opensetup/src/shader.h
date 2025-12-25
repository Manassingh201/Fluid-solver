#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>

class Shader {
public:
    GLuint program;

    Shader(const char* vertexSource, const char* fragmentSource);
    ~Shader();

    void use() const;
    void setInt(const char* name, int value) const;
    void setFloat(const char* name, float value) const;
    void setVec2(const char* name, float x, float y) const;
    void setVec3(const char* name, float x, float y, float z) const;

private:
    GLuint compileShader(GLenum type, const char* source);
    GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);
};

#endif