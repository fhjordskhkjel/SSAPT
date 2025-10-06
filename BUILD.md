# Building SSAPT

## Prerequisites

- Windows 10 or later
- Visual Studio 2019 or later with C++ development tools
- CMake 3.10 or later
- Windows SDK (for DirectX headers)
- Microsoft Detours library (optional, for IAT hooking)

## Build Instructions

### Using CMake (Recommended)

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Generate build files:
```bash
cmake ..
```

3. Build the project:
```bash
cmake --build . --config Release
```

The output will be:
- `ssapt.dll` - The main driver library
- `ssapt_test.exe` - Test executable

### Using Visual Studio

1. Open Visual Studio
2. Select "Open a local folder" and choose the SSAPT directory
3. CMake integration will automatically configure the project
4. Build using Ctrl+Shift+B or Build menu

## Manual Build (without CMake)

If you prefer to build manually:

```bash
cl /LD /EHsc /std:c++17 driver.cpp dx_hooks.cpp /link gdi32.lib d3d9.lib d3d11.lib dxgi.lib /OUT:ssapt.dll
cl /EHsc /std:c++17 test_driver.cpp /link gdi32.lib ssapt.lib /OUT:ssapt_test.exe
```

## Testing

After building, run the test executable:

```bash
cd build
.\ssapt_test.exe
```

The test will attempt to take screenshots with blocking enabled and disabled, verifying that the driver works correctly.

## Installation

To use the driver in your own applications:

1. Copy `ssapt.dll` to your application directory
2. Include `driver.h` in your source code
3. Link against `ssapt.lib` (or load the DLL dynamically)

## Usage Example

```cpp
#include "driver.h"

int main() {
    // Enable screenshot blocking
    EnableBlocking();
    
    // Your code here - screenshots will be blocked
    
    // Disable blocking when done
    DisableBlocking();
    
    return 0;
}
```

## Troubleshooting

### Build Errors

- **Missing DirectX headers**: Install the Windows SDK
- **Detours not found**: The driver works without Detours but uses vtable hooking instead
- **Linker errors**: Ensure you're linking against the required libraries (gdi32, d3d9, d3d11, dxgi)

### Runtime Issues

- **Hooks not working**: Run the application with administrator privileges
- **Access denied**: Some antivirus software may flag the driver - add an exception if needed
- **DirectX hooks not applied**: The driver hooks DirectX at runtime and may not catch all applications

## Notes

- This driver is designed for testing and educational purposes
- Administrator privileges may be required for some hooking methods
- The driver hooks at the application level, not kernel level
- Some screenshot tools may use alternative methods not covered by these hooks
