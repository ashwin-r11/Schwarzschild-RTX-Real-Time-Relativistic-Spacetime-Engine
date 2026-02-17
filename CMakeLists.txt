cmake_minimum_required(VERSION 3.10)
project(BlackHoleSim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source directory as include path
include_directories(${PROJECT_SOURCE_DIR}/src)

# Add tests
enable_testing()
add_subdirectory(tests)