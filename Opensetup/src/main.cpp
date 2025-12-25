#include "Application.h"
#include <iostream>

int main() {
    Application app(800, 800, "GPU Fluid Simulation");

    if (!app.initialize()) {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }

    app.run();

    return 0;
}