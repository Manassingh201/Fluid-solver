#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <GLFW/glfw3.h>

class InputHandler {
public:
    InputHandler();

    void update();
    void setWindow(GLFWwindow* window);

    bool isMouseDown() const { return mouseDown; }
    double getMouseX() const { return mouseX; }
    double getMouseY() const { return mouseY; }
    double getPrevMouseX() const { return prevMouseX; }
    double getPrevMouseY() const { return prevMouseY; }
    double getMouseDeltaX() const { return mouseX - prevMouseX; }
    double getMouseDeltaY() const { return mouseY - prevMouseY; }

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

private:
    double mouseX, mouseY;
    double prevMouseX, prevMouseY;
    bool mouseDown;

    static InputHandler* instance;
};

#endif