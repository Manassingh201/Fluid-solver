# GPU Fluid Simulation (Lagrangian Solver)

A real-time **GPU-accelerated fluid simulation** implemented in **C++ and OpenGL**, using a **Lagrangian-style particle-based approach** combined with full-screen shader passes for visualization.

This project explores interactive fluid dynamics, smooth advection, and visually rich flow behavior entirely on the GPU.

---

##  Features

-  **Real-time GPU fluid simulation**
-  **Lagrangian particle-based motion**
-  Smooth, glowing fluid visualization using shaders
-  Interactive input-driven forces
-  Modular simulation pipeline
-  High-performance OpenGL rendering

---

## 🖼️ Screenshots

### Blue Fluid Flow
![Blue Fluid](screenshots/Screenshot (25).png)

### Purple / White Flow Interaction
![Purple Fluid](screenshots/Screenshot (25).png)

### Yellow Energy Burst
![Yellow Fluid](screenshots/Screenshot (25).png)

---

##  Simulation Overview

The solver follows a **Lagrangian approach**, where fluid behavior is represented by particles that:

1. Are advected through a velocity field
2. Accumulate forces from user input
3. Are visualized via GPU shaders
4. Produce smooth, continuous flow patterns

All heavy computation and rendering are performed on the **GPU**, enabling real-time interaction.

