# Math tests
add_executable(vec3_test math/vec3_test.cpp)
add_test(NAME Vec3Test COMMAND vec3_test)

add_executable(vec4_test math/vec4_test.cpp)
add_test(NAME Vec4Test COMMAND vec4_test)

# Physics engine tests (no GPU/display required)
add_executable(physics_test physics/physics_test.cpp)
add_test(NAME PhysicsTest COMMAND physics_test)