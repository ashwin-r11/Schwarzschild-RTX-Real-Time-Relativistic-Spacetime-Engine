#include <thread>
#include <vector>
#include <cstdint>
#include <iostream>

namespace Renderer {

    // This is the function each thread will run independently
    inline void renderScreenBands(std::vector<uint32_t>& pixels, int width, int start_y, int end_y) {
        
        for (int y = start_y; y < end_y; ++y) {
            for (int x = 0; x < width; ++x) {
                
                // 1. TODO: Convert (x, y) to a 3D ray (Camera Math)
                
                // 2. TODO: Call Physics::tracePhoton(ray)
                
                // 3. TODO: Convert HitRecord to a color
                uint32_t pixel_color = 0xFF0000FF; // Example: Pure Red
                
                // 4. Write to the 1D pixel array
                pixels[y * width + x] = pixel_color;
            }
        }
    }

    // The manager function that divides the work
    inline void renderFrame(std::vector<uint32_t>& pixels, int width, int height) {
        
        // Find out how many cores the CPU has
        unsigned int num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) num_threads = 4; // Fallback just in case
        
        std::vector<std::jthread> threads;
        int rows_per_thread = height / num_threads;

        // Spawn the threads and assign them their rows
        for (unsigned int i = 0; i < num_threads; ++i) {
            
            int start_y = i * rows_per_thread;
            // The last thread picks up any leftover rows
            int end_y = (i == num_threads - 1) ? height : start_y + rows_per_thread;

            // Emplace a new C++20 jthread. 
            // We pass the function, and then the arguments it needs.
            threads.emplace_back(renderScreenBands, std::ref(pixels), width, start_y, end_y);
        }

    }
}