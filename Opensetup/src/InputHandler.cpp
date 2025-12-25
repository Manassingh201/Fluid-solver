#include "InputHandler.h"

InputHandler* InputHandler::instance = nullptr;

InputHandler::InputHandler()
    : mouseX(0.0), mouseY(0.0), prevMouseX(0.0), prevMouseY(0.0), mouseDown(false) {
    instance = this;
}

void InputHandler::update() {
    if (mouseDown) {
        prevMouseX = mouseX;
        prevMouseY = mouseY;
    }
}

void InputHandler::setWindow(GLFWwindow* window) {
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
}

void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (instance && button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            instance->mouseDown = true;
            glfwGetCursorPos(window, &instance->prevMouseX, &instance->prevMouseY);
            instance->mouseX = instance->prevMouseX;
            instance->mouseY = instance->prevMouseY;
        }
        else if (action == GLFW_RELEASE) {
            instance->mouseDown = false;
        }
    }
}

void InputHandler::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    if (instance) {
        instance->mouseX = xpos;
        instance->mouseY = ypos;
    }
}