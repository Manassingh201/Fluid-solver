#include "Application.h"
#include <glad/glad.h>
#include <iostream>
#include <algorithm>
#include <cmath>

Application::Application(int width, int height, const char* title)
    : windowWidth(width), windowHeight(height), windowTitle(title),
    window(nullptr), lastTime(0.0), fpsTime(0.0), frameCount(0) {
}

Application::~Application() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

bool Application::initialize() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // Create input handler and set up callbacks
    inputHandler = std::make_unique<InputHandler>();
    inputHandler->setWindow(window);

    // Create and initialize fluid simulation
    fluidSim = std::make_unique<FluidSimulation>(512, 512);
    fluidSim->init();

    lastTime = glfwGetTime();
    fpsTime = lastTime;

    return true;
}

void Application::run() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double currentTime = glfwGetTime();
        float dt = (float)(currentTime - lastTime);
        lastTime = currentTime;

        updateFPS();

        // Clamp dt to prevent instability
        dt = std::min(dt, 0.016f);

        processInput(dt);

        // Step simulation
        fluidSim->step(dt);

        // Render
        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        fluidSim->render(winWidth, winHeight);

        glfwSwapBuffers(window);
    }
}

void Application::processInput(float dt) {
    if (inputHandler->isMouseDown()) {
        int winWidth, winHeight;
        glfwGetWindowSize(window, &winWidth, &winHeight);

        float x = (float)inputHandler->getMouseX() / winWidth;
        float y = 1.0f - (float)inputHandler->getMouseY() / winHeight;

        float dx = (float)inputHandler->getMouseDeltaX() / winWidth;
        float dy = -(float)inputHandler->getMouseDeltaY() / winHeight;

        // Add force based on mouse movement
        fluidSim->addForce(x, y, dx * 10.0f, dy * 10.0f);

        // Add colorful dye
        float time = (float)glfwGetTime();
        float r = 0.5f + 0.5f * sin(time * 2.0f);
        float g = 0.5f + 0.5f * sin(time * 3.0f + 1.0f);
        float b = 0.5f + 0.5f * sin(time * 4.0f + 2.0f);

        fluidSim->addDye(x, y, r * 0.8f, g * 0.8f, b * 0.8f);

        inputHandler->update();
    }
}

void Application::updateFPS() {
    frameCount++;
    double currentTime = glfwGetTime();
    if (currentTime - fpsTime >= 1.0) {
        double fps = frameCount / (currentTime - fpsTime);
        std::cout << "FPS: " << (int)fps << " | Frame time: " << (1000.0 / fps) << "ms" << std::endl;
        frameCount = 0;
        fpsTime = currentTime;
    }
}

void Application::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}