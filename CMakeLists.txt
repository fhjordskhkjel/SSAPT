cmake_minimum_required(VERSION 3.10)
project(SSAPT VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Control application (replaces DLL-based control)
add_executable(ssapt_control
    control_app.cpp
)

# Set output name
set_target_properties(ssapt_control PROPERTIES
    OUTPUT_NAME "ssapt"
)

# Installation
install(TARGETS ssapt_control
    RUNTIME DESTINATION bin
)

# Note: Kernel driver (ssapt.sys) must be built using Windows Driver Kit (WDK)
# Use the 'sources' file with the WDK build environment
# Or use Visual Studio with WDK integration

message(STATUS "Building SSAPT Control Application")
message(STATUS "Kernel driver must be built separately using WDK")
message(STATUS "See BUILD.md for detailed instructions")
