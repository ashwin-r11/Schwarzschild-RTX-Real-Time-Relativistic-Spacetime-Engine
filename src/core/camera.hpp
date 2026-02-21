#pragma once

#include "../math/Vec3.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Camera {
public:
    vec3 position;
    vec3 forward;
    vec3 right;
    vec3 up;
    double fov_scale;

    // Constructor: Takes where you are, and where you are looking
    Camera(vec3 pos, vec3 look_at, double fov_degrees = 90.0) {
        position = pos;
        
        // 1. Calculate Basis Vectors (The Camera Matrix equivalent)
        vec3 world_up(0.0, 1.0, 0.0);
        
        forward = (look_at - position).normalize();
        right = forward.cross(world_up).normalize();
        up = right.cross(forward).normalize();
        
        // 2. Calculate Field of View scaling
        double fov_radians = fov_degrees * M_PI / 180.0;
        fov_scale = std::tan(fov_radians * 0.5);
    }

    // Generates a 3D ray direction for a specific 2D pixel
    vec3 getRayDirection(double u, double v, double aspect_ratio) const {
        // u and v are normalized screen coordinates from -1.0 to 1.0
        vec3 pixel_dir = forward + 
                         right * (u * fov_scale * aspect_ratio) + 
                         up * (v * fov_scale);
        
        return pixel_dir.normalize();
    }
};