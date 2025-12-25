#ifndef APPLICATION_H
#define APPLICATION_H

#include "FluidSimulation.h"
#include "InputHandler.h"
#include <GLFW/glfw3.h>
#include <memory>


class Application {
public:
    Application(int width, int height, const char* title);
    ~Application();

    bool initialize();
    void run();

private:
    int windowWidth, windowHeight;
    const char* windowTitle;
    GLFWwindow* window;

    std::unique_ptr<FluidSimulation> fluidSim;
    std::unique_ptr<InputHandler> inputHandler;

    double lastTime;
    double fpsTime;
    int frameCount;

    void processInput(float dt);
    void updateFPS();

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};

#endif