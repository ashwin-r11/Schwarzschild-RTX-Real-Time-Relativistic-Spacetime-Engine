cmake_minimum_required(VERSION 3.10)
project(BlackHoleSim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source directory as include path
include_directories(${PROJECT_SOURCE_DIR}/src)

# GLAD - OpenGL loader
add_library(glad ${PROJECT_SOURCE_DIR}/third_party/glad/src/glad.c)
target_include_directories(glad PUBLIC ${PROJECT_SOURCE_DIR}/third_party/glad/include)

# Find GLFW
find_package(glfw3 3.3 REQUIRED)

# Main executable
add_executable(BlackHoleSim src/main.cpp)
target_link_libraries(BlackHoleSim glad glfw dl)

# Add tests
enable_testing()
add_subdirectory(tests)