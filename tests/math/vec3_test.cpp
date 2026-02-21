#include "math/Vec3.hpp"
#include <cmath>
#include <iostream>
#include <string>

// ============================================================
//  Minimal assertion framework (no external dependencies)
// ============================================================
static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT_EQ(val, expected, name) \
    if (std::abs((val) - (expected)) < 1e-9) { \
        tests_passed++; \
    } else { \
        tests_failed++; \
        std::cerr << "  FAIL: " << name << " — expected " << expected << ", got " << val << "\n"; \
    }

#define ASSERT_VEC3_EQ(v, ex, ey, ez, name) \
    if (std::abs((v).x - (ex)) < 1e-9 && \
        std::abs((v).y - (ey)) < 1e-9 && \
        std::abs((v).z - (ez)) < 1e-9) { \
        tests_passed++; \
    } else { \
        tests_failed++; \
        std::cerr << "  FAIL: " << name << " — expected (" << ex << ", " << ey << ", " << ez \
                  << "), got (" << (v).x << ", " << (v).y << ", " << (v).z << ")\n"; \
    }

int main() {
    std::cout << "=== Vec3 Unit Tests ===\n\n";

    vec3 a(1, 2, 3);
    vec3 b(4, 5, 6);

    // --- Addition ---
    vec3 sum = a + b;
    ASSERT_VEC3_EQ(sum, 5, 7, 9, "Addition");

    // --- Subtraction ---
    vec3 diff = b - a;
    ASSERT_VEC3_EQ(diff, 3, 3, 3, "Subtraction");

    // --- Scalar multiplication ---
    vec3 scaled = a * 2.0;
    ASSERT_VEC3_EQ(scaled, 2, 4, 6, "Scalar multiplication");

    // --- Scalar division ---
    vec3 divided = b / 2.0;
    ASSERT_VEC3_EQ(divided, 2, 2.5, 3, "Scalar division");

    // --- Dot product ---
    double d = a.dot(b);
    ASSERT_EQ(d, 32.0, "Dot product");

    // --- Cross product: X × Y = Z ---
    vec3 cx = vec3(1, 0, 0).cross(vec3(0, 1, 0));
    ASSERT_VEC3_EQ(cx, 0, 0, 1, "Cross product X×Y");

    // --- Cross product: Y × X = -Z ---
    vec3 cy = vec3(0, 1, 0).cross(vec3(1, 0, 0));
    ASSERT_VEC3_EQ(cy, 0, 0, -1, "Cross product Y×X");

    // --- Length (3-4-5 triangle) ---
    vec3 c(0, 3, 4);
    ASSERT_EQ(c.length(), 5.0, "Length 3-4-5");

    // --- Normalize ---
    vec3 n = c.normalize();
    ASSERT_EQ(n.length(), 1.0, "Normalized length = 1");
    ASSERT_VEC3_EQ(n, 0.0, 0.6, 0.8, "Normalized components");

    // --- Zero vector length ---
    vec3 z(0, 0, 0);
    ASSERT_EQ(z.length(), 0.0, "Zero vector length");

    // --- Self dot product = length² ---
    double selfDot = a.dot(a);
    double lenSq = a.length() * a.length();
    ASSERT_EQ(selfDot, lenSq, "Self dot = length²");

    // --- Cross product is perpendicular to both inputs ---
    vec3 p(1, 2, 3), q(4, -1, 2);
    vec3 cr = p.cross(q);
    ASSERT_EQ(cr.dot(p), 0.0, "Cross ⊥ first input");
    ASSERT_EQ(cr.dot(q), 0.0, "Cross ⊥ second input");

    // --- Negative scalar ---
    vec3 neg = a * -1.0;
    ASSERT_VEC3_EQ(neg, -1, -2, -3, "Negative scalar");

    // --- Summary ---
    std::cout << "\nResults: " << tests_passed << " passed, " << tests_failed << " failed\n";
    return tests_failed > 0 ? 1 : 0;
}