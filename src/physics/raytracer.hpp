#pragma once

#include "../math/Vec3.hpp"
#include <cmath>

namespace Physics {
    
    // Natural Units
    const double G = 1.0;
    const double M = 1.0;
    const double C = 1.0;
    const double RS = 2.0; // Schwarzschild Radius
    const double ESCAPE_RADIUS = 20.0;
    const double STEP_SIZE = 0.05; // How far the photon moves per step (dt)

    // --- NEW: Accretion Disk Dimensions ---
    const double DISK_INNER = 2.6; // Just outside the event horizon
    const double DISK_OUTER = 12.0;

    // A simple struct to hold our photon's state
    struct Photon {
        vec3 pos;
        vec3 vel;
    };

    enum class HitTarget {
        BLACK_HOLE,
        BACKGROUND_SKY,
        ACCRETION_DISK
    };

    struct HitRecord {
        HitTarget target;
        // We can add color or temperature data here later!
    };

    // Module 03: The Schwarzschild Acceleration
    inline vec3 calculateAcceleration(const vec3& pos, const vec3& vel) {
        double r2 = pos.dot(pos); 
        double r = std::sqrt(r2);
        vec3 h_vec = pos.cross(vel);
        double h2 = h_vec.dot(h_vec);
        double r5 = r2 * r2 * r;
        
        return pos * (-3.0 * M * h2 / r5);
    }

    // Module 04: The RK4 Integrator
    inline void stepRK4(Photon& p, double dt) {
        // Sample 1 (Start)
        vec3 k1_vel = calculateAcceleration(p.pos, p.vel);
        vec3 k1_pos = p.vel;

        // Sample 2 (Midpoint using k1)
        vec3 k2_vel = calculateAcceleration(p.pos + k1_pos * (dt * 0.5), p.vel + k1_vel * (dt * 0.5));
        vec3 k2_pos = p.vel + k1_vel * (dt * 0.5);

        // Sample 3 (Midpoint using k2)
        vec3 k3_vel = calculateAcceleration(p.pos + k2_pos * (dt * 0.5), p.vel + k2_vel * (dt * 0.5));
        vec3 k3_pos = p.vel + k2_vel * (dt * 0.5);

        // Sample 4 (Endpoint using k3)
        vec3 k4_vel = calculateAcceleration(p.pos + k3_pos * dt, p.vel + k3_vel * dt);
        vec3 k4_pos = p.vel + k3_vel * dt;

        // Combine the samples and take the actual step
        p.vel = p.vel + (k1_vel + k2_vel * 2.0 + k3_vel * 2.0 + k4_vel) * (dt / 6.0);
        p.pos = p.pos + (k1_pos + k2_pos * 2.0 + k3_pos * 2.0 + k4_pos) * (dt / 6.0);
    }

    // The Main Raytracing Loop (Returns true if it hit the black hole, false if it escaped)
    inline HitRecord tracePhoton(Photon p) {
        
        // Loop until it crashes or escapes
        while (true) {
            // SAVE THIS BEFORE THE STEP!
            double old_y = p.pos.y; 

            double r = p.pos.length();

            // Capture condition
            if (r <= RS) {
                return { HitTarget::BLACK_HOLE }; // Fixed return
            }

            // Escape condition
            if (r > ESCAPE_RADIUS) {
                return { HitTarget::BACKGROUND_SKY }; // Fixed return
            }

            // Move the photon forward one tick
            stepRK4(p, STEP_SIZE);
        
            double new_y = p.pos.y;
            
            // Did we cross the Y=0 plane?
            if ((old_y > 0.0 && new_y <= 0.0) || (old_y < 0.0 && new_y >= 0.0)) {
                
                // We crossed the plane! Now check if we are within the disk's rings.
                double radius_on_disk = std::sqrt(p.pos.x * p.pos.x + p.pos.z * p.pos.z);
                    
                if (radius_on_disk >= DISK_INNER && radius_on_disk <= DISK_OUTER) {
                    return { HitTarget::ACCRETION_DISK };
                }
            }
        }
    }
}