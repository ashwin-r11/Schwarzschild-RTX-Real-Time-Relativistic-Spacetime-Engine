#include <iostream>
#include <vector>

// Include your custom engine core components
#include "core/display.hpp"
#include "core/camera.hpp"
#include "core/renderer.hpp"

const int WIDTH = 800;
const int HEIGHT = 600;

int main() {
    std::cout << "Starting General Relativity Engine...\n";

    // 1. Initialize Display Window (This wakes up GLFW, GLAD, and the PBO/FBO)
    Display display(WIDTH, HEIGHT, "Schwarzschild Black Hole");

    // 2. Initialize Pixel Array (1D vector representing 2D screen)
    // We start it completely black: 0xFF000000 (Alpha, Blue, Green, Red)
    std::vector<uint32_t> pixels(WIDTH * HEIGHT, 0xFF000000); 

    // 3. Initialize Camera (Positioned slightly up and far back, looking at the center 0,0,0)
    Camera camera(vec3(0.0, 3.0, 15.0), vec3(0.0, 0.0, 0.0), 90.0);

    // 4. Main Render Loop
    while (!display.shouldClose()) {
        
        // --- INPUT HANDLING ---
        double speed = 0.5;
        // Move Forward/Backward along the Z axis
        if (display.isKeyPressed(GLFW_KEY_W)) camera.position = camera.position + camera.forward * speed;
        if (display.isKeyPressed(GLFW_KEY_S)) camera.position = camera.position - camera.forward * speed;
        
        // Strafe Left/Right along the X axis
        if (display.isKeyPressed(GLFW_KEY_D)) camera.position = camera.position + camera.right * speed;
        if (display.isKeyPressed(GLFW_KEY_A)) camera.position = camera.position - camera.right * speed;
        
        // Fly Up/Down along the Y axis
        if (display.isKeyPressed(GLFW_KEY_E)) camera.position = camera.position + camera.up * speed;
        if (display.isKeyPressed(GLFW_KEY_Q)) camera.position = camera.position - camera.up * speed;

        // --- PHYSICS & RENDERING ---
        // Fire the photons across all CPU cores to calculate curved spacetime
        Renderer::renderFrame(pixels, WIDTH, HEIGHT, camera);

        // --- GPU UPLOAD ---
        // Take the calculated colors and blast them to the screen instantly
        display.update(pixels.data());
    }

    std::cout << "Engine Shutting Down...\n";
    return 0;
}