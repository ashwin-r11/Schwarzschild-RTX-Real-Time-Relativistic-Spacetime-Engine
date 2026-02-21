#pragma once

#include "../math/Vec3.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================
//  Spherical Orbit Camera — CAD-style controls
//  State: yaw, pitch, radius around an orbit center
//  No gimbal lock: pitch clamped to ±89°
// ============================================================
class Camera {
public:
    // Spherical parameters
    float yaw;        // Horizontal angle (radians)
    float pitch;      // Vertical angle (radians), clamped
    float radius;     // Distance from orbit center

    // Orbit target (the point we revolve around)
    vec3 center;

    // Derived basis vectors (recomputed each frame)
    vec3 position;
    vec3 forward;
    vec3 right;
    vec3 up;

    // FOV
    float fov_scale;

    // Input sensitivity
    float mouse_sensitivity;
    float scroll_sensitivity;
    float move_speed;

    // Mouse state
    bool dragging;
    double lastMouseX, lastMouseY;

    Camera(float init_radius = 15.0f, float init_yaw = 0.0f, float init_pitch = 0.3f)
        : yaw(init_yaw), pitch(init_pitch), radius(init_radius),
          center(0.0, 0.0, 0.0),
          mouse_sensitivity(0.005f),
          scroll_sensitivity(1.2f),
          move_speed(0.3f),
          dragging(false),
          lastMouseX(0.0), lastMouseY(0.0)
    {
        // 90° FOV
        double fov_radians = 90.0 * M_PI / 180.0;
        fov_scale = static_cast<float>(std::tan(fov_radians * 0.5));
        update();
    }

    // Recompute position & basis vectors from spherical coordinates
    void update() {
        // Clamp pitch to avoid gimbal lock at poles
        float maxPitch = static_cast<float>(89.0 * M_PI / 180.0);
        if (pitch > maxPitch)  pitch = maxPitch;
        if (pitch < -maxPitch) pitch = -maxPitch;

        // Clamp radius
        if (radius < 2.5f) radius = 2.5f;
        if (radius > 200.0f) radius = 200.0f;

        // Spherical to Cartesian
        float cosP = std::cos(pitch);
        float sinP = std::sin(pitch);
        float cosY = std::cos(yaw);
        float sinY = std::sin(yaw);

        position.x = center.x + radius * cosP * sinY;
        position.y = center.y + radius * sinP;
        position.z = center.z + radius * cosP * cosY;

        // Look-at direction
        forward = (center - position).normalize();

        // Camera basis
        vec3 world_up(0.0, 1.0, 0.0);
        right = forward.cross(world_up).normalize();
        up = right.cross(forward).normalize();
    }

    // Called on mouse button press/release
    void onMouseButton(int button, int action) {
        if (button == 0) { // Left mouse button (GLFW_MOUSE_BUTTON_LEFT)
            dragging = (action == 1); // GLFW_PRESS
        }
    }

    // Called on mouse move
    void onMouseMove(double xpos, double ypos) {
        if (dragging) {
            double dx = xpos - lastMouseX;
            double dy = ypos - lastMouseY;

            yaw   -= static_cast<float>(dx) * mouse_sensitivity;
            pitch += static_cast<float>(dy) * mouse_sensitivity;
        }
        lastMouseX = xpos;
        lastMouseY = ypos;
    }

    // Called on scroll
    void onScroll(double yoffset) {
        radius -= static_cast<float>(yoffset) * scroll_sensitivity;
    }

    // Process WASD + Q/E for panning the orbit center
    void processKeyboard(bool w, bool s, bool a, bool d, bool q, bool e) {
        // Pan in the local right/forward/up directions (projected to XZ plane)
        vec3 pan_forward = vec3(forward.x, 0.0, forward.z).normalize();
        vec3 pan_right   = vec3(right.x, 0.0, right.z).normalize();

        if (w) center = center + pan_forward * move_speed;
        if (s) center = center - pan_forward * move_speed;
        if (d) center = center + pan_right * move_speed;
        if (a) center = center - pan_right * move_speed;
        if (e) center = center + vec3(0.0, 1.0, 0.0) * move_speed;
        if (q) center = center - vec3(0.0, 1.0, 0.0) * move_speed;
    }
};