#include "math/Vec4.hpp"
#include <cmath>
#include <iostream>

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT_EQ(val, expected, name) \
    if (std::abs((val) - (expected)) < 1e-9) { \
        tests_passed++; \
    } else { \
        tests_failed++; \
        std::cerr << "  FAIL: " << name << " — expected " << expected << ", got " << val << "\n"; \
    }

#define ASSERT_VEC4_EQ(v, ex, ey, ez, ew, name) \
    if (std::abs((v).x - (ex)) < 1e-9 && \
        std::abs((v).y - (ey)) < 1e-9 && \
        std::abs((v).z - (ez)) < 1e-9 && \
        std::abs((v).w - (ew)) < 1e-9) { \
        tests_passed++; \
    } else { \
        tests_failed++; \
        std::cerr << "  FAIL: " << name << " — expected (" << ex << ", " << ey << ", " << ez << ", " << ew \
                  << "), got (" << (v).x << ", " << (v).y << ", " << (v).z << ", " << (v).w << ")\n"; \
    }

int main() {
    std::cout << "=== Vec4 Unit Tests ===\n\n";

    vec4 a(1, 2, 3, 1);
    vec4 b(4, 5, 6, 1);

    // --- Addition ---
    ASSERT_VEC4_EQ(a + b, 5, 7, 9, 2, "Addition");

    // --- Subtraction ---
    ASSERT_VEC4_EQ(b - a, 3, 3, 3, 0, "Subtraction");

    // --- Scalar multiplication ---
    ASSERT_VEC4_EQ(a * 2.0, 2, 4, 6, 2, "Scalar multiplication");

    // --- Scalar division ---
    ASSERT_VEC4_EQ(b / 2.0, 2, 2.5, 3, 0.5, "Scalar division");

    // --- Dot product ---
    ASSERT_EQ(a.dot(b), 33.0, "Dot product");

    // --- Cross product (w should be 0) ---
    vec4 cx = vec4(1,0,0,0).cross(vec4(0,1,0,0));
    ASSERT_VEC4_EQ(cx, 0, 0, 1, 0, "Cross X×Y (w=0)");

    // --- Length (ignores w) ---
    vec4 c(0, 3, 4, 0);
    ASSERT_EQ(c.length(), 5.0, "Length 3-4-5");

    // --- Normalize ---
    vec4 n = c.normalize();
    ASSERT_EQ(n.length(), 1.0, "Normalized length = 1");

    // --- Homogeneous: Point - Point = Direction (w=0) ---
    vec4 p1(10, 10, 10, 1);
    vec4 p2(5, 5, 5, 1);
    ASSERT_VEC4_EQ(p1 - p2, 5, 5, 5, 0, "Point - Point = Direction");

    // --- Homogeneous: Point + Direction = Point (w=1) ---
    vec4 dir(0, 0, -5, 0);
    ASSERT_VEC4_EQ(p1 + dir, 10, 10, 5, 1, "Point + Direction = Point");

    // --- Summary ---
    std::cout << "\nResults: " << tests_passed << " passed, " << tests_failed << " failed\n";
    return tests_failed > 0 ? 1 : 0;
}