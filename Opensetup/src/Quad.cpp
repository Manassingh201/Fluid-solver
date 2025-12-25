#include "Quad.h"

void Quad::init() {
    float q[] = {
        -1,-1,0,0, 1,-1,1,0, 1,1,1,1,
        -1,-1,0,0, 1,1,1,1, -1,1,0,1
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(q), q, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Quad::draw() const {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
