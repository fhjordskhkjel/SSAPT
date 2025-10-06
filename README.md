# SSAPT - Screenshot Anti-Protection Testing

A Windows driver for private testing that blocks screenshots by hooking into GDI and DirectX rendering paths and blocking access to frame buffers.

## Overview

SSAPT is a lightweight DLL-based driver that intercepts common screenshot capture methods on Windows systems. It works by hooking into:

1. **GDI (Graphics Device Interface)** rendering paths
   - BitBlt operations
   - GetDIBits frame buffer access
   - Compatible DC and bitmap creation

2. **DirectX rendering paths**
   - DirectX 9 Present and GetFrontBufferData
   - DirectX 11/DXGI Present and GetBuffer
   - Frame buffer access blocking

## Features

- ✅ Blocks GDI-based screenshot tools (BitBlt, PrintScreen)
- ✅ Blocks DirectX frame buffer access
- ✅ Runtime enable/disable of blocking
- ✅ Minimal performance overhead
- ✅ Easy integration via DLL
- ✅ Comprehensive test suite

## Quick Start

### Building

See [BUILD.md](BUILD.md) for detailed build instructions.

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Testing

Run the included test suite:

```bash
.\build\ssapt_test.exe
```

### Usage

Load the DLL in your application:

```cpp
#include "driver.h"

// Enable screenshot blocking
EnableBlocking();

// Your protected code here...

// Disable blocking when done
DisableBlocking();
```

## How It Works

### GDI Hooking

The driver hooks critical GDI functions using IAT (Import Address Table) hooking or detours:

- **BitBlt**: Blocks the primary method used by most screenshot tools
- **GetDIBits**: Prevents direct frame buffer reading
- **CreateCompatibleDC/Bitmap**: Monitors bitmap operations

### DirectX Hooking

DirectX methods are hooked via vtable manipulation:

- **D3D9 Present**: Monitors frame presentation (allows normal rendering)
- **D3D9 GetFrontBufferData**: Blocks front buffer capture
- **DXGI Present**: Monitors DXGI swap chain presentation
- **DXGI GetBuffer**: Blocks buffer access for screenshots

### Architecture

```
Application
    ↓
SSAPT Driver (DLL)
    ↓
Hooked Functions → Original Functions (if allowed)
    ↓
Windows GDI/DirectX
```

## API Reference

### Functions

- `void EnableBlocking()` - Enables screenshot blocking
- `void DisableBlocking()` - Disables screenshot blocking
- `bool IsBlockingEnabled()` - Checks if blocking is currently enabled

### Export

All functions are exported with C linkage for easy FFI integration.

## Limitations

- Application-level hooking only (not kernel-level)
- May not block all screenshot methods (e.g., hardware capture cards)
- Requires the DLL to be loaded in the target process
- Some antivirus software may flag hooking behavior

## Use Cases

- Testing DRM and copy protection systems
- Educational purposes for understanding Windows graphics APIs
- Security research and vulnerability assessment
- Private content protection during testing

## Technical Details

### Hooked Functions

**GDI Functions:**
- `BitBlt` - Blocks screen-to-memory transfers
- `GetDIBits` - Blocks DIB (Device Independent Bitmap) retrieval
- `CreateCompatibleDC` - Monitored for screenshot detection
- `CreateCompatibleBitmap` - Monitored for screenshot detection

**DirectX 9 Functions:**
- `IDirect3DDevice9::Present` - Monitored (allows normal rendering)
- `IDirect3DDevice9::GetFrontBufferData` - Blocked

**DirectX 11/DXGI Functions:**
- `IDXGISwapChain::Present` - Monitored (allows normal rendering)
- `IDXGISwapChain::GetBuffer` - Blocked

## Contributing

This is a private testing repository. Contributions should follow secure coding practices and be thoroughly tested.

## License

For private testing use only.

## Security Notice

This driver is designed for testing purposes. Using it in production environments or to circumvent legitimate security measures may violate terms of service or local laws.