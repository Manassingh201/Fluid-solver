#include "Shader.h"
#include <iostream>

Shader::Shader(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    program = linkProgram(vertexShader, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    glDeleteProgram(program);
}

void Shader::use() const {
    glUseProgram(program);
}

void Shader::setInt(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(program, name), value);
}

void Shader::setFloat(const char* name, float value) const {
    glUniform1f(glGetUniformLocation(program, name), value);
}

void Shader::setVec2(const char* name, float x, float y) const {
    glUniform2f(glGetUniformLocation(program, name), x, y);
}

void Shader::setVec3(const char* name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(program, name), x, y, z);
}

GLuint Shader::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "Shader compilation failed: " << infoLog << std::endl;
    }
    return shader;
}

GLuint Shader::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vertexShader);
    glAttachShader(prog, fragmentShader);
    glLinkProgram(prog);

    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(prog, 512, NULL, infoLog);
        std::cout << "Program linking failed: " << infoLog << std::endl;
    }
    return prog;
}