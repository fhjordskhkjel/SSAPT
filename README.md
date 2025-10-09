# SSAPT - Screenshot Anti-Protection Testing

A Windows kernel-mode driver for system-wide screenshot blocking that hooks into kernel graphics APIs.

## Overview

SSAPT is a kernel-mode driver that provides system-wide screenshot protection by intercepting screenshot capture methods at the kernel level. Unlike user-mode solutions, this driver operates at the kernel level for comprehensive protection.

**Key Features:**

1. **10 kernel-mode hooks** for comprehensive screenshot blocking
2. **Kernel-mode protection** for system-wide coverage
3. **User-mode control application** for easy management
4. **No DLL injection required** - works system-wide
5. **IOCTL-based communication** between user-mode and kernel
6. **Comprehensive BSOD protection** with exception handling and parameter validation

## Architecture

```
User Mode:
  ssapt.exe (Control Application)
       |
       | IOCTL Commands
       ▼
Kernel Mode:
  ssapt.sys (Kernel Driver)
       |
       | Hooks kernel graphics APIs
       ▼
  Windows Kernel Graphics Subsystem
```

## Features

- ✅ **System-wide screenshot blocking** at kernel level
- ✅ **Standalone control application** (no DLL required)
- ✅ Runtime enable/disable via simple commands
- ✅ Minimal performance overhead
- ✅ **Kernel-level safety checks** to prevent system crashes
- ✅ **Structured exception handling** in kernel code
- ✅ **Spin lock protection** for thread-safe state management

## Quick Start

### Building

See [BUILD.md](BUILD.md) for detailed build instructions.

**Prerequisites:**
- Windows Driver Kit (WDK) for kernel driver
- Visual Studio 2019 or later
- Administrator privileges for installation

**Build Control Application:**
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

**Build Kernel Driver:**
Use Visual Studio with WDK integration or the WDK build environment.

### Installation

**Step 1: Install the kernel driver**
```cmd
# Run as Administrator
driver_manager.bat install
driver_manager.bat start
```

**Step 2: Verify driver is running**
```cmd
driver_manager.bat status
```

### Usage

**Enable screenshot blocking:**
```cmd
ssapt.exe enable
```

**Disable screenshot blocking:**
```cmd
ssapt.exe disable
```

**Check status:**
```cmd
ssapt.exe status
```

## How It Works

### Kernel-Mode Hooking

The kernel driver uses **SSDT (System Service Descriptor Table) hooking** to intercept graphics-related system calls at the kernel level:

**SSDT Hooking Process:**
1. Locates the System Service Descriptor Table in kernel memory
2. Disables memory write protection (CR0 register manipulation)
3. Replaces function pointers in SSDT with custom hook functions
4. Re-enables write protection
5. Forwards calls to original functions when appropriate

**Target Functions:**
- **NtGdiBitBlt**: GDI bit block transfers (main screenshot method)
- **NtGdiStretchBlt**: Stretched bit block transfers
- **NtGdiGetDIBitsInternal**: Direct DIB pixel reading
- **NtUserGetDC/NtUserGetWindowDC**: Device context retrieval (monitoring)
- **NtGdiCreateCompatibleDC/Bitmap**: Compatible DC/bitmap creation (monitoring)
- **NtUserPrintWindow**: Print window to bitmap
- **NtGdiDdDDIPresent**: DirectX frame presentation (monitoring)
- **NtGdiDdDDIGetDisplayModeList**: Display mode enumeration

See [SSDT_IMPLEMENTATION.md](SSDT_IMPLEMENTATION.md) for detailed technical information about the SSDT hooking implementation.

### Communication Architecture

```
User Mode Control App (ssapt.exe)
    ↓ (IOCTL Commands)
Kernel Driver (ssapt.sys)
    ↓ (Hook/Block)
Windows Kernel Graphics APIs
    ↓
Display Hardware
```

### IOCTL Interface

The control application communicates with the kernel driver via Device I/O Control (IOCTL):

- **IOCTL_SSAPT_ENABLE**: Enable system-wide blocking
- **IOCTL_SSAPT_DISABLE**: Disable system-wide blocking  
- **IOCTL_SSAPT_STATUS**: Query current blocking state

## Command Reference

### Control Application Commands

- `ssapt.exe enable` - Enables system-wide screenshot blocking
- `ssapt.exe disable` - Disables system-wide screenshot blocking
- `ssapt.exe status` - Checks if blocking is currently enabled
- `ssapt.exe help` - Shows usage information

### Driver Management

- `driver_manager.bat install` - Installs the kernel driver
- `driver_manager.bat start` - Starts the driver service
- `driver_manager.bat stop` - Stops the driver service
- `driver_manager.bat uninstall` - Uninstalls the driver
- `driver_manager.bat status` - Shows driver status

## Limitations

- Requires Windows Driver Signature (or test signing mode for development)
- Must be installed and run with Administrator privileges
- May not block hardware capture cards or physical camera capture
- Requires system reboot in some cases for driver changes
- Only works on Windows operating systems

## Safety and Stability

The kernel driver includes comprehensive safety measures:

- **Spin Lock Protection**: Thread-safe state management using kernel spin locks
- **IRQL Management**: Proper IRQL level handling for all operations
- **Structured Exception Handling**: Kernel-safe exception handling
- **Memory Validation**: All pointers validated before use in kernel space
- **Graceful Error Handling**: Returns proper NTSTATUS codes

These protections ensure the driver operates safely in kernel mode without causing system crashes or BSODs.

## Use Cases

- Testing DRM and copy protection systems
- Educational purposes for understanding Windows graphics APIs
- Security research and vulnerability assessment
- Private content protection during testing

## Technical Details

### Kernel APIs Hooked

**Windows Graphics Kernel APIs (10 hooks total):**
- `NtGdiDdDDIPresent` - Monitors DirectX frame presentation (allows normal rendering)
- `NtGdiDdDDIGetDisplayModeList` - Blocks display mode enumeration when protection is enabled
- `NtGdiBitBlt` - Blocks large GDI bit block transfers (>100x100 pixels)
- `NtGdiStretchBlt` - Blocks large stretched bit block transfers (>100x100 pixels)
- `NtUserGetDC` - Monitors device context retrieval
- `NtUserGetWindowDC` - Monitors window device context retrieval
- `NtGdiGetDIBitsInternal` - Blocks direct DIB pixel reading operations
- `NtGdiCreateCompatibleDC` - Monitors compatible DC creation (screenshot preparation)
- `NtGdiCreateCompatibleBitmap` - Monitors compatible bitmap creation (screenshot preparation)
- `NtUserPrintWindow` - Blocks window screenshot operations via PrintWindow API

### Driver Architecture

**Kernel Components:**
- Device object: `\Device\SSAPT`
- Symbolic link: `\??\SSAPT`
- Service name: `SSAPT`
- Start type: Demand start

**User-Mode Components:**
- Control application: `ssapt.exe`
- Device link: `\\.\SSAPT`

## Contributing

This is a private testing repository. Contributions should follow secure coding practices and be thoroughly tested.

## License

For private testing use only.

## Security Notice

This kernel driver is designed for testing and research purposes only. 

**Important Warnings:**
- Kernel-mode code runs with highest privileges and can affect system stability
- Only install drivers from trusted sources
- Use test signing mode during development
- Driver must be properly signed for production use
- Using this to circumvent legitimate security measures may violate laws or terms of service
- Always test in a controlled environment first

## Development Mode

To install unsigned drivers during development:

1. Enable test signing mode:
```cmd
bcdedit /set testsigning on
```

2. Reboot the system

3. Install and test the driver

4. When done, disable test signing:
```cmd
bcdedit /set testsigning off
```