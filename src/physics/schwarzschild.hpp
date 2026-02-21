
#pragma once

#include "../math/Vec3.hpp"
#include <cmath>

namespace Physics {
    
    // Natural Units Constants
    const double G = 1.0;
    const double M = 1.0;
    const double C = 1.0;
    const double SCHWARZSCHILD_RADIUS = 2.0 * G * M / (C * C); // Equals 2.0

    // Calculates the gravitational "acceleration" bending the photon's path
    inline vec3 calculateAcceleration(const vec3& pos, const vec3& vel) {
        
        // 1. Calculate r (distance from center) and r^2
        double r2 = pos.dot(pos); 
        double r = std::sqrt(r2);
        
        // 2. Calculate angular momentum vector (h = p x v)
        vec3 h_vec = pos.cross(vel);
        
        // 3. Get the squared magnitude of angular momentum (h^2)
        double h2 = h_vec.dot(h_vec);
        
        // 4. Calculate r^5
        double r5 = r2 * r2 * r;
        
        // 5. Compute the Schwarzschild acceleration vector
        // Formula: a = - (3 * M * h^2 / r^5) * pos
        double term = -3.0 * M * h2 / r5;
        
        return pos * term;
    }
}