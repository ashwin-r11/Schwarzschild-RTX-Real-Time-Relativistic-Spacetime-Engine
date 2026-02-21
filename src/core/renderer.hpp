#pragma once

#include <thread>
#include <vector>
#include <cstdint>
#include "camera.hpp"               // <-- Brings in the Camera
#include "../physics/raytracer.hpp" // <-- Brings in the Physics

namespace Renderer {

    inline void renderScreenBands(std::vector<uint32_t>& pixels, int width, int height, int start_y, int end_y, const Camera& camera) {
        
        double aspect_ratio = (double)width / (double)height;

        for (int y = start_y; y < end_y; ++y) {
            for (int x = 0; x < width; ++x) {
                
                // 1. Convert pixel (x,y) to normalized coordinates (-1.0 to 1.0)
                double u = (2.0 * (x + 0.5) / width) - 1.0;
                double v = 1.0 - (2.0 * (y + 0.5) / height); 

                // 2. Spawn the photon using the Camera!
                Physics::Photon photon;
                photon.pos = camera.position;
                photon.vel = camera.getRayDirection(u, v, aspect_ratio);
                
                // 3. Trace it through curved spacetime!
                Physics::HitRecord hit = Physics::tracePhoton(photon);
                
                // 4. Convert what we hit into a pixel color
                uint32_t pixel_color = 0xFF000000; // Default Black

                if (hit.target == Physics::HitTarget::BLACK_HOLE) {
                    pixel_color = 0xFF000000; // Pure Pitch Black
                } 
                else if (hit.target == Physics::HitTarget::ACCRETION_DISK) {
                    pixel_color = 0xFF00A5FF; // Interstellar Orange/Yellow
                } 
                else if (hit.target == Physics::HitTarget::BACKGROUND_SKY) {
                    pixel_color = 0xFF221111; // Very dark space blue
                }
                
                // 5. Write to the 1D pixel array
                pixels[y * width + x] = pixel_color;
            }
        }
    }

    // Notice we added 'const Camera& camera' here!
    inline void renderFrame(std::vector<uint32_t>& pixels, int width, int height, const Camera& camera) {
        unsigned int num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) num_threads = 4;
        
        std::vector<std::jthread> threads;
        int rows_per_thread = height / num_threads;

        for (unsigned int i = 0; i < num_threads; ++i) {
            int start_y = i * rows_per_thread;
            int end_y = (i == num_threads - 1) ? height : start_y + rows_per_thread;

            // Pass the camera into the threads using std::cref
            threads.emplace_back(renderScreenBands, std::ref(pixels), width, height, start_y, end_y, std::cref(camera));
        }
    }
}