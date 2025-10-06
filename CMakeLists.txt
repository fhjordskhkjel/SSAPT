cmake_minimum_required(VERSION 3.10)
project(SSAPT VERSION 2.0.0 LANGUAGES CXX)

# ============================================================================
# SSAPT - Screenshot Anti-Protection Testing
# ============================================================================
# This CMake file builds the USER-MODE control application only.
# The KERNEL-MODE driver (ssapt.sys) must be built separately using WDK.
# ============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# ============================================================================
# Control Application (User-Mode)
# ============================================================================
# This application communicates with the kernel driver via IOCTL commands
# to enable/disable screenshot blocking system-wide.

add_executable(ssapt_control
    control_app.cpp
)

# Set output name to 'ssapt' (without .exe on Linux/Mac for cross-platform)
set_target_properties(ssapt_control PROPERTIES
    OUTPUT_NAME "ssapt"
)

# Link Windows libraries
if(WIN32)
    # No additional libraries needed - using Windows API directly
    target_compile_definitions(ssapt_control PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
    )
endif()

# Installation
install(TARGETS ssapt_control
    RUNTIME DESTINATION bin
    COMPONENT runtime
)

# ============================================================================
# Build Instructions
# ============================================================================
message(STATUS "")
message(STATUS "=======================================================")
message(STATUS "SSAPT Build Configuration")
message(STATUS "=======================================================")
message(STATUS "Building: User-mode control application (ssapt.exe)")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "IMPORTANT: This builds ONLY the control application!")
message(STATUS "")
message(STATUS "The kernel driver (ssapt.sys) must be built separately:")
message(STATUS "  1. Install Windows Driver Kit (WDK)")
message(STATUS "  2. Use Visual Studio with WDK integration")
message(STATUS "  3. Or use WDK command-line build environment")
message(STATUS "")
message(STATUS "Quick Start:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  cmake --build . --config Release")
message(STATUS "")
message(STATUS "See BUILD.md for complete build and installation guide")
message(STATUS "=======================================================")
message(STATUS "")

# ============================================================================
# Kernel Driver Build Information
# ============================================================================
# The kernel driver is located in: kernel_driver.c
# Build methods:
#   1. Visual Studio + WDK: Open as Driver project
#   2. WDK CLI: Use 'sources' file with build command
#   3. MSBuild: msbuild ssapt.vcxproj /p:Configuration=Release
#
# After building kernel driver:
#   - Output: ssapt.sys (kernel driver)
#   - Install: driver_manager.bat install
#   - Start: driver_manager.bat start
#   - Control: ssapt.exe enable|disable|status
# ============================================================================
