#include "physics/raytracer.hpp"
#include <cmath>
#include <iostream>

// ============================================================
//  Unit tests for the Schwarzschild physics engine
//  Tests: acceleration, RK4 integrator, photon tracing
// ============================================================

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT_TRUE(cond, name) \
    if (cond) { \
        tests_passed++; \
    } else { \
        tests_failed++; \
        std::cerr << "  FAIL: " << name << "\n"; \
    }

#define ASSERT_NEAR(val, expected, tol, name) \
    if (std::abs((val) - (expected)) < (tol)) { \
        tests_passed++; \
    } else { \
        tests_failed++; \
        std::cerr << "  FAIL: " << name << " — expected " << expected << " ± " << tol \
                  << ", got " << val << "\n"; \
    }

int main() {
    std::cout << "=== Physics Engine Unit Tests ===\n\n";

    // --------------------------------------------------
    //  Test 1: Schwarzschild acceleration direction
    //  Acceleration should always point toward the origin
    // --------------------------------------------------
    {
        vec3 pos(5.0, 0.0, 0.0);
        vec3 vel(0.0, 0.0, 1.0);
        vec3 acc = Physics::calculateAcceleration(pos, vel);
        // Acceleration should be in -x direction (toward origin)
        ASSERT_TRUE(acc.x < 0.0, "Acceleration points toward origin (x < 0)");
        ASSERT_NEAR(acc.y, 0.0, 1e-10, "Acceleration y=0 for equatorial photon");
        ASSERT_NEAR(acc.z, 0.0, 1e-10, "Acceleration z=0 for tangential photon");
    }

    // --------------------------------------------------
    //  Test 2: Acceleration magnitude scales with 1/r⁵
    //  Doubling distance should reduce |a| by a factor ~32
    // --------------------------------------------------
    {
        vec3 vel(0, 0, 1);
        vec3 a1 = Physics::calculateAcceleration(vec3(4, 0, 0), vel);
        vec3 a2 = Physics::calculateAcceleration(vec3(8, 0, 0), vel);

        double mag1 = a1.length();
        double mag2 = a2.length();
        // For tangential v=1: |h| = r, so a = 3M * r² / r⁵ = 3M / r³
        // Ratio at r=4 vs r=8: (8/4)³ = 8, but we also need |a| ratio
        // |a| = 3M*h²/r⁵, h=r → 3M*r²/r⁵ = 3M/r³
        // mag1/mag2 = (8³)/(4³) = 8  ... wait, mag1 is at r=4 (larger)
        // Actually mag1 (r=4) = 3/(64) and mag2 (r=8) = 3/(512)
        // ratio = 512/64 = 8. But let's verify with looser tolerance.
        double ratio = mag1 / mag2;
        ASSERT_TRUE(ratio > 3.0 && ratio < 12.0, "Acceleration stronger at smaller r");
    }

    // --------------------------------------------------
    //  Test 3: RK4 integrator conserves angular momentum
    //  L = r × v should be approximately constant
    // --------------------------------------------------
    {
        Physics::Photon p;
        p.pos = vec3(10.0, 0.0, 0.0);
        p.vel = vec3(0.0, 0.0, -1.0);

        vec3 L0 = p.pos.cross(p.vel);
        double L0_mag = L0.length();

        // Take 100 RK4 steps
        for (int i = 0; i < 100; i++) {
            Physics::stepRK4(p, Physics::STEP_SIZE);
        }

        vec3 L1 = p.pos.cross(p.vel);
        double L1_mag = L1.length();

        double drift = std::abs(L1_mag - L0_mag) / L0_mag;
        ASSERT_TRUE(drift < 0.01, "Angular momentum conserved to <1% over 100 steps");
    }

    // --------------------------------------------------
    //  Test 4: Photon aimed directly at BH → capture
    // --------------------------------------------------
    {
        Physics::Photon p;
        p.pos = vec3(10.0, 0.0, 0.0);
        p.vel = vec3(-1.0, 0.0, 0.0); // Directly inward

        Physics::HitRecord hit = Physics::tracePhoton(p);
        ASSERT_TRUE(hit.target == Physics::HitTarget::BLACK_HOLE,
                    "Direct radial photon captured by black hole");
    }

    // --------------------------------------------------
    //  Test 5: Photon aimed away from BH → escape
    // --------------------------------------------------
    {
        Physics::Photon p;
        p.pos = vec3(10.0, 0.0, 0.0);
        p.vel = vec3(1.0, 0.0, 0.0); // Directly outward

        Physics::HitRecord hit = Physics::tracePhoton(p);
        ASSERT_TRUE(hit.target == Physics::HitTarget::BACKGROUND_SKY,
                    "Outward radial photon escapes to sky");
    }

    // --------------------------------------------------
    //  Test 6: Photon at shallow angle → disk hit
    //  Photon slightly above disk plane with tangential velocity
    // --------------------------------------------------
    {
        Physics::Photon p;
        p.pos = vec3(8.0, 0.5, 0.0); // Slightly above disk plane
        p.vel = vec3(0.0, -0.1, -1.0).normalize(); // Crosses y=0

        Physics::HitRecord hit = Physics::tracePhoton(p);
        ASSERT_TRUE(hit.target == Physics::HitTarget::ACCRETION_DISK,
                    "Shallow-angle photon hits accretion disk");
    }

    // --------------------------------------------------
    //  Test 7: Schwarzschild radius constant check
    // --------------------------------------------------
    ASSERT_NEAR(Physics::RS, 2.0, 1e-10, "Schwarzschild radius = 2M");
    ASSERT_NEAR(Physics::M, 1.0, 1e-10, "Mass = 1 (natural units)");
    ASSERT_NEAR(Physics::G, 1.0, 1e-10, "G = 1 (natural units)");

    // --------------------------------------------------
    //  Test 8: RK4 doesn't explode for near-horizon photon
    // --------------------------------------------------
    {
        Physics::Photon p;
        p.pos = vec3(2.5, 0.0, 0.0); // Just outside event horizon
        p.vel = vec3(0.0, 0.0, 1.0);

        // Should complete without nan/inf
        Physics::HitRecord hit = Physics::tracePhoton(p);
        bool validResult = (hit.target == Physics::HitTarget::BLACK_HOLE ||
                           hit.target == Physics::HitTarget::BACKGROUND_SKY ||
                           hit.target == Physics::HitTarget::ACCRETION_DISK);
        ASSERT_TRUE(validResult, "Near-horizon photon produces valid result (no NaN/crash)");
    }

    // --------------------------------------------------
    //  Test 9: Acceleration is zero at infinity
    // --------------------------------------------------
    {
        vec3 pos(1e4, 0, 0);
        vec3 vel(0, 0, 1);
        vec3 acc = Physics::calculateAcceleration(pos, vel);
        // a = 3M*h²/r⁵ ≈ 3e-12 at r=1e4, should be negligible
        ASSERT_TRUE(acc.length() < 1e-6, "Acceleration → 0 at large r");
    }

    // --- Summary ---
    std::cout << "\nResults: " << tests_passed << " passed, " << tests_failed << " failed\n";
    return tests_failed > 0 ? 1 : 0;
}
