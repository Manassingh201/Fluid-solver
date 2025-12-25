#pragma once
#include <glad/glad.h>

class Quad {
public:
    Quad() = default;
    void init();     // <-- new
    void draw() const;

private:
    GLuint vao = 0;
    GLuint vbo = 0;
};
