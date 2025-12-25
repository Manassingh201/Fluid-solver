#ifndef FLUIDSIMULATION_H
#define FLUIDSIMULATION_H

#include <glad/glad.h>
#include <memory>
#include "Shader.h"

class FluidSimulation {
public:
    FluidSimulation(int width, int height);
    ~FluidSimulation();

    void init();
    void step(float dt);
    void render(int windowWidth, int windowHeight);
    void addForce(float x, float y, float fx, float fy);
    void addDye(float x, float y, float r, float g, float b);

private:
    int gridW, gridH;

    // Textures
    GLuint velocityTextures[2];
    GLuint dyeTextures[2];
    GLuint pressureTextures[2];
    GLuint divergenceTexture;
    GLuint vorticityTexture;

    // Framebuffers
    GLuint framebuffers[2];

    // Shaders
    std::unique_ptr<Shader> advectShader;
    std::unique_ptr<Shader> divergenceShader;
    std::unique_ptr<Shader> pressureShader;
    std::unique_ptr<Shader> gradientShader;
    std::unique_ptr<Shader> splatShader;
    std::unique_ptr<Shader> displayShader;
    std::unique_ptr<Shader> vorticityShader;
    std::unique_ptr<Shader> confinementShader;

    // VAO
    GLuint quadVAO;

    // Buffer indices
    int currentVel;
    int currentDye;
    int currentPressure;

    // Private methods
    void createShaders();
    void createTexturePair(GLuint textures[2], GLenum internalFormat, GLenum format, GLenum type);
    void setupTexture(GLuint texture, GLenum internalFormat, GLenum format, GLenum type);
    GLuint createQuadVAO();
    void initVelocityField();

    void bindFramebuffer(GLuint texture);
    void unbindFramebuffer();

    void advectVelocity(float dt);
    void advectDye(float dt);
    void computeDivergence();
    void solvePressure(int iterations = 20);
    void computeVorticity();
    void applyVorticityConfinement(float dt);
    void subtractGradient();
};

#endif