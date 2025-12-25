#include "FluidSimulation.h"
#include "ShaderSources.h"
#include <cmath>
#include <vector>

FluidSimulation::FluidSimulation(int width, int height)
    : gridW(width), gridH(height), currentVel(0), currentDye(0), currentPressure(0) {
}

FluidSimulation::~FluidSimulation() {
    glDeleteTextures(2, velocityTextures);
    glDeleteTextures(2, dyeTextures);
    glDeleteTextures(2, pressureTextures);
    glDeleteTextures(1, &divergenceTexture);
    glDeleteTextures(1, &vorticityTexture);
    glDeleteFramebuffers(2, framebuffers);
    glDeleteVertexArrays(1, &quadVAO);
}

void FluidSimulation::init() {
    createShaders();

    glGenFramebuffers(2, framebuffers);

    createTexturePair(velocityTextures, GL_RG32F, GL_RG, GL_FLOAT);
    createTexturePair(dyeTextures, GL_RGB32F, GL_RGB, GL_FLOAT);
    createTexturePair(pressureTextures, GL_R32F, GL_RED, GL_FLOAT);

    glGenTextures(1, &divergenceTexture);
    setupTexture(divergenceTexture, GL_R32F, GL_RED, GL_FLOAT);

    glGenTextures(1, &vorticityTexture);
    setupTexture(vorticityTexture, GL_R32F, GL_RED, GL_FLOAT);

    quadVAO = createQuadVAO();
    initVelocityField();
}

void FluidSimulation::createShaders() {
    advectShader = std::make_unique<Shader>(ShaderSources::vs_shader, ShaderSources::advect_fs);
    divergenceShader = std::make_unique<Shader>(ShaderSources::vs_shader, ShaderSources::divergence_fs);
    pressureShader = std::make_unique<Shader>(ShaderSources::vs_shader, ShaderSources::pressure_fs);
    gradientShader = std::make_unique<Shader>(ShaderSources::vs_shader, ShaderSources::gradient_fs);
    splatShader = std::make_unique<Shader>(ShaderSources::vs_shader, ShaderSources::splat_fs);
    displayShader = std::make_unique<Shader>(ShaderSources::vs_shader, ShaderSources::display_fs);
    vorticityShader = std::make_unique<Shader>(ShaderSources::vs_shader, ShaderSources::vorticity_fs);
    confinementShader = std::make_unique<Shader>(ShaderSources::vs_shader, ShaderSources::confinement_fs);
}

void FluidSimulation::createTexturePair(GLuint textures[2], GLenum internalFormat, GLenum format, GLenum type) {
    for (int i = 0; i < 2; i++) {
        glGenTextures(1, &textures[i]);
        setupTexture(textures[i], internalFormat, format, type);
    }
}

void FluidSimulation::setupTexture(GLuint texture, GLenum internalFormat, GLenum format, GLenum type) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, gridW, gridH, 0, format, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

GLuint FluidSimulation::createQuadVAO() {
    float quad[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    return vao;
}

void FluidSimulation::initVelocityField() {
    std::vector<float> velData(gridW * gridH * 2);
    for (int j = 0; j < gridH; j++) {
        for (int i = 0; i < gridW; i++) {
            float x = (i + 0.5f) / gridW * 2.0f - 1.0f;
            float y = (j + 0.5f) / gridH * 2.0f - 1.0f;
            float len = sqrt(x * x + y * y) + 0.001f;

            int idx = (j * gridW + i) * 2;
            velData[idx + 0] = y / len * 0.1f;
            velData[idx + 1] = -x / len * 0.1f;
        }
    }

    glBindTexture(GL_TEXTURE_2D, velocityTextures[0]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gridW, gridH, GL_RG, GL_FLOAT, velData.data());
    glBindTexture(GL_TEXTURE_2D, velocityTextures[1]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gridW, gridH, GL_RG, GL_FLOAT, velData.data());
}

void FluidSimulation::bindFramebuffer(GLuint texture) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glViewport(0, 0, gridW, gridH);
}

void FluidSimulation::unbindFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FluidSimulation::advectVelocity(float dt) {
    bindFramebuffer(velocityTextures[1 - currentVel]);

    advectShader->use();
    advectShader->setFloat("dt", dt * 50.0f);
    advectShader->setVec2("texelSize", 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocityTextures[currentVel]);
    advectShader->setInt("field", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, velocityTextures[currentVel]);
    advectShader->setInt("velocity", 1);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    currentVel = 1 - currentVel;
    unbindFramebuffer();
}

void FluidSimulation::computeDivergence() {
    bindFramebuffer(divergenceTexture);

    divergenceShader->use();
    divergenceShader->setVec2("texelSize", 1.0f / gridW, 1.0f / gridH);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocityTextures[currentVel]);
    divergenceShader->setInt("velocity", 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    unbindFramebuffer();
}

void FluidSimulation::solvePressure(int iterations) {
    float alpha = -1.0f;
    float beta = 0.25f;

    bindFramebuffer(pressureTextures[currentPressure]);
    glClear(GL_COLOR_BUFFER_BIT);
    unbindFramebuffer();

    for (int i = 0; i < iterations; i++) {
        bindFramebuffer(pressureTextures[1 - currentPressure]);

        pressureShader->use();
        pressureShader->setVec2("texelSize", 1.0f / gridW, 1.0f / gridH);
        pressureShader->setFloat("alpha", alpha);
        pressureShader->setFloat("beta", beta);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pressureTextures[currentPressure]);
        pressureShader->setInt("pressure", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, divergenceTexture);
        pressureShader->setInt("divergence", 1);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        currentPressure = 1 - currentPressure;
        unbindFramebuffer();
    }
}

void FluidSimulation::computeVorticity() {
    bindFramebuffer(vorticityTexture);

    vorticityShader->use();
    vorticityShader->setVec2("texelSize", 1.0f / gridW, 1.0f / gridH);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocityTextures[currentVel]);
    vorticityShader->setInt("velocity", 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    unbindFramebuffer();
}

void FluidSimulation::applyVorticityConfinement(float dt) {
    bindFramebuffer(velocityTextures[1 - currentVel]);

    confinementShader->use();
    confinementShader->setVec2("texelSize", 1.0f / gridW, 1.0f / gridH);
    confinementShader->setFloat("dt", dt);
    confinementShader->setFloat("strength", 0.3f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocityTextures[currentVel]);
    confinementShader->setInt("velocity", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, vorticityTexture);
    confinementShader->setInt("vorticity", 1);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    currentVel = 1 - currentVel;
    unbindFramebuffer();
}

void FluidSimulation::subtractGradient() {
    bindFramebuffer(velocityTextures[1 - currentVel]);

    gradientShader->use();
    gradientShader->setVec2("texelSize", 1.0f / gridW, 1.0f / gridH);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocityTextures[currentVel]);
    gradientShader->setInt("velocity", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pressureTextures[currentPressure]);
    gradientShader->setInt("pressure", 1);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    currentVel = 1 - currentVel;
    unbindFramebuffer();
}

void FluidSimulation::advectDye(float dt) {
    bindFramebuffer(dyeTextures[1 - currentDye]);

    advectShader->use();
    advectShader->setFloat("dt", dt * 50.0f);
    advectShader->setVec2("texelSize", 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dyeTextures[currentDye]);
    advectShader->setInt("field", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, velocityTextures[currentVel]);
    advectShader->setInt("velocity", 1);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    currentDye = 1 - currentDye;
    unbindFramebuffer();
}

void FluidSimulation::addForce(float x, float y, float fx, float fy) {
    bindFramebuffer(velocityTextures[1 - currentVel]);

    splatShader->use();
    splatShader->setVec2("point", x * gridW, y * gridH);
    splatShader->setVec3("color", fx, fy, 0.0f);
    splatShader->setFloat("radius", 200.0f);
    splatShader->setFloat("strength", 0.05f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocityTextures[currentVel]);
    splatShader->setInt("base", 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    currentVel = 1 - currentVel;
    unbindFramebuffer();
}

void FluidSimulation::addDye(float x, float y, float r, float g, float b) {
    bindFramebuffer(dyeTextures[1 - currentDye]);

    splatShader->use();
    splatShader->setVec2("point", x * gridW, y * gridH);
    splatShader->setVec3("color", r, g, b);
    splatShader->setFloat("radius", 100.0f);
    splatShader->setFloat("strength", 0.8f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dyeTextures[currentDye]);
    splatShader->setInt("base", 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    currentDye = 1 - currentDye;
    unbindFramebuffer();
}

void FluidSimulation::step(float dt) {
    advectVelocity(dt);
    computeVorticity();
    applyVorticityConfinement(dt);
    computeDivergence();
    solvePressure(20);
    subtractGradient();
    advectDye(dt);
}

void FluidSimulation::render(int windowWidth, int windowHeight) {
    glViewport(0, 0, windowWidth, windowHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    displayShader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dyeTextures[currentDye]);
    displayShader->setInt("tex", 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}