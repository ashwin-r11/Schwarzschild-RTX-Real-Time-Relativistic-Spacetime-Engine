// ============================================================
//  Schwarzschild Black Hole — Phase 5: GPU Fragment Shader
//  All physics runs in blackhole.frag on the RTX 4070
//  CPU only: pass uniforms → draw 2 triangles → done
// ============================================================

#include <iostream>
#include <cmath>
#include <string>

#include "core/display.hpp"
#include "core/camera.hpp"

const int WIDTH  = 800;
const int HEIGHT = 600;

// Global camera pointer for GLFW callbacks
Camera* g_camera = nullptr;

// --- GLFW Callbacks ---
void mouseButtonCallback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) {
    if (g_camera) g_camera->onMouseButton(button, action);
}

void cursorPosCallback(GLFWwindow* /*window*/, double xpos, double ypos) {
    if (g_camera) g_camera->onMouseMove(xpos, ypos);
}

void scrollCallback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset) {
    if (g_camera) g_camera->onScroll(yoffset);
}

int main() {
    std::cout << "===================================\n";
    std::cout << " Schwarzschild Black Hole Engine\n";
    std::cout << " Phase 5: GPU Fragment Shader\n";
    std::cout << "===================================\n\n";

    // Resolve shader paths relative to executable
    // Shaders are in src/shaders/ relative to project root
    std::string vertPath = "../src/shaders/blackhole.vert";
    std::string fragPath = "../src/shaders/blackhole.frag";

    // 1. Initialize Display (compiles shaders, creates fullscreen quad)
    Display display(WIDTH, HEIGHT, "Schwarzschild Black Hole", vertPath, fragPath);

    // 2. Initialize Orbit Camera
    Camera camera(15.0f, 0.0f, 0.3f);  // radius=15, yaw=0, pitch=0.3 rad (~17°)
    g_camera = &camera;

    // 3. Register input callbacks
    GLFWwindow* win = display.getWindow();
    glfwSetMouseButtonCallback(win, mouseButtonCallback);
    glfwSetCursorPosCallback(win, cursorPosCallback);
    glfwSetScrollCallback(win, scrollCallback);

    // ESC to close
    glfwSetKeyCallback(win, [](GLFWwindow* w, int key, int, int action, int) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(w, GLFW_TRUE);
    });

    std::cout << "Controls:\n";
    std::cout << "  Mouse Drag : Orbit around black hole\n";
    std::cout << "  Scroll     : Zoom in/out\n";
    std::cout << "  WASD       : Pan orbit center\n";
    std::cout << "  Q/E        : Move center up/down\n";
    std::cout << "  ESC        : Quit\n\n";

    float time = 0.0f;

    // 4. Main Render Loop — CPU does almost nothing!
    while (!display.shouldClose()) {

        // --- Process keyboard input ---
        camera.processKeyboard(
            display.isKeyPressed(GLFW_KEY_W),
            display.isKeyPressed(GLFW_KEY_S),
            display.isKeyPressed(GLFW_KEY_A),
            display.isKeyPressed(GLFW_KEY_D),
            display.isKeyPressed(GLFW_KEY_Q),
            display.isKeyPressed(GLFW_KEY_E)
        );

        // Recalculate camera basis from spherical coordinates
        camera.update();

        // --- Pass uniforms to GPU ---
        display.setUniform2f("uResolution", (float)display.getWidth(), (float)display.getHeight());
        display.setUniform1f("uTime", time);
        display.setUniform1f("uStepSize", 0.08f);
        display.setUniform1f("uFovScale", camera.fov_scale);

        display.setUniform3f("uCamPos",
            (float)camera.position.x,
            (float)camera.position.y,
            (float)camera.position.z);

        display.setUniform3f("uCamForward",
            (float)camera.forward.x,
            (float)camera.forward.y,
            (float)camera.forward.z);

        display.setUniform3f("uCamRight",
            (float)camera.right.x,
            (float)camera.right.y,
            (float)camera.right.z);

        display.setUniform3f("uCamUp",
            (float)camera.up.x,
            (float)camera.up.y,
            (float)camera.up.z);

        // --- Draw! The GPU does ALL the physics ---
        display.draw();

        time += 0.016f; // ~60fps time accumulator
    }

    std::cout << "\nEngine Shutting Down...\n";
    return 0;
}