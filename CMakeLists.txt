cmake_minimum_required(VERSION 3.10)
project(SSAPT VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main driver library
add_library(ssapt_driver SHARED
    driver.cpp
    dx_hooks.cpp
    driver.h
)

# Link against required Windows libraries
target_link_libraries(ssapt_driver PRIVATE
    gdi32
    d3d9
    d3d11
    dxgi
)

# Set output name
set_target_properties(ssapt_driver PROPERTIES
    OUTPUT_NAME "ssapt"
    PREFIX ""
)

# Test executable
add_executable(ssapt_test
    test_driver.cpp
)

target_link_libraries(ssapt_test PRIVATE
    ssapt_driver
    gdi32
)

# Example application
add_executable(ssapt_example
    example.cpp
)

target_link_libraries(ssapt_example PRIVATE
    ssapt_driver
    gdi32
)

# Safety test executable
add_executable(ssapt_safety_test
    test_safety.cpp
)

target_link_libraries(ssapt_safety_test PRIVATE
    ssapt_driver
    gdi32
)

# Installation
install(TARGETS ssapt_driver
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES driver.h
    DESTINATION include
)
