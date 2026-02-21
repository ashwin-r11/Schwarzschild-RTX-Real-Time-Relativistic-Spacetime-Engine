// ============================================================
//  Schwarzschild Black Hole — Phase 5: GPU Fragment Shader
//  Multi-pass HDR bloom pipeline on RTX 4070
//  CPU only: pass uniforms → draw 2 triangles → bloom → done
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
void mouseButtonCallback(GLFWwindow*, int button, int action, int) {
    if (g_camera) g_camera->onMouseButton(button, action);
}

void cursorPosCallback(GLFWwindow*, double xpos, double ypos) {
    if (g_camera) g_camera->onMouseMove(xpos, ypos);
}

void scrollCallback(GLFWwindow*, double, double yoffset) {
    if (g_camera) g_camera->onScroll(yoffset);
}

int main() {
    std::cout << "===================================\n";
    std::cout << " Schwarzschild Black Hole Engine\n";
    std::cout << " Phase 5: GPU + HDR Bloom\n";
    std::cout << "===================================\n\n";

    // Shader directory (relative to build/)
    std::string shaderDir = "../src/shaders";

    // 1. Initialize Display (compiles 3 shader programs, creates bloom FBOs)
    Display display(WIDTH, HEIGHT, "Schwarzschild Black Hole", shaderDir);

    // 2. Initialize Orbit Camera
    Camera camera(15.0f, 0.0f, 0.3f);
    g_camera = &camera;

    // 3. Register input callbacks
    GLFWwindow* win = display.getWindow();
    glfwSetMouseButtonCallback(win, mouseButtonCallback);
    glfwSetCursorPosCallback(win, cursorPosCallback);
    glfwSetScrollCallback(win, scrollCallback);

    glfwSetKeyCallback(win, [](GLFWwindow* w, int key, int, int action, int) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(w, GLFW_TRUE);
    });

    std::cout << "Controls:\n";
    std::cout << "  Mouse Drag  : Orbit around black hole\n";
    std::cout << "  Scroll      : Zoom in/out\n";
    std::cout << "  WASD        : Pan orbit center\n";
    std::cout << "  Q/E         : Move center up/down\n";
    std::cout << "  +/-         : Adjust bloom strength\n";
    std::cout << "  ESC         : Quit\n\n";

    float time = 0.0f;

    // 4. Main Render Loop
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

        camera.update();

        // --- Activate scene shader and set uniforms ---
        display.useSceneShader();
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

        // --- Draw! Scene → Bloom → Composite → Screen ---
        display.draw();

        time += 0.016f;
    }

    std::cout << "\nEngine Shutting Down...\n";
    return 0;
}