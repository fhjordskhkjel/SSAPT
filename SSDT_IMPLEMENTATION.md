# SSDT Hooking Implementation Guide

## Overview

This document explains the SSDT (System Service Descriptor Table) hooking implementation in the SSAPT kernel driver and provides guidance for configuring service indexes for different Windows versions.

## What is SSDT Hooking?

The System Service Descriptor Table (SSDT) is a kernel-mode data structure that contains pointers to kernel service routines. When a user-mode application makes a system call, Windows uses the SSDT to dispatch the call to the appropriate kernel function.

SSDT hooking works by:
1. Locating the SSDT in kernel memory
2. Disabling memory write protection (CR0 register manipulation)
3. Replacing function pointers in the SSDT with custom hook functions
4. Re-enabling write protection
5. Forwarding calls to original functions when needed

## Implementation Components

### 1. SSDT Structure Definition (lines 64-74)

```c
typedef struct _SERVICE_DESCRIPTOR_TABLE {
    PVOID* ServiceTableBase;        // Array of function pointers
    PULONG ServiceCounterTableBase; // Call counters (optional)
    ULONG NumberOfServices;         // Number of services in table
    PUCHAR ParamTableBase;          // Parameter info (optional)
} SERVICE_DESCRIPTOR_TABLE, *PSERVICE_DESCRIPTOR_TABLE;

extern PSERVICE_DESCRIPTOR_TABLE KeServiceDescriptorTable;
```

### 2. Write Protection Control (lines 460-482)

**DisableWriteProtection()** - Clears the WP (Write Protect) bit in CR0 register:
```c
VOID DisableWriteProtection(VOID) {
    ULONG_PTR cr0 = __readcr0();
    cr0 &= ~0x10000;  // Clear bit 16 (WP)
    __writecr0(cr0);
}
```

**EnableWriteProtection()** - Sets the WP bit back:
```c
VOID EnableWriteProtection(VOID) {
    ULONG_PTR cr0 = __readcr0();
    cr0 |= 0x10000;   // Set bit 16 (WP)
    __writecr0(cr0);
}
```

### 3. SSDT Hook Installation (lines 516-563)

**SetSSDTHook()** - Replaces an SSDT entry with a hook:
```c
BOOLEAN SetSSDTHook(ULONG serviceIndex, PVOID hookFunction, PVOID* originalFunction)
```

This function:
- Validates all parameters and SSDT structure
- Validates service index is within bounds
- Stores the original function pointer
- Disables write protection
- Replaces SSDT entry with hook function
- Re-enables write protection
- Uses SEH (Structured Exception Handling) for safety

### 4. Hook Initialization (lines 566-744)

**InitializeHooks()** - Sets up all SSDT hooks:
- Validates KeServiceDescriptorTable is available
- Logs SSDT location and size
- Attempts to hook each target function
- Counts successful hook installations
- Provides detailed logging for debugging
- Continues even if some hooks fail (graceful degradation)

### 5. Hook Removal (lines 746-862)

**RemoveHooks()** - Restores original SSDT entries:
- Validates SSDT is still available
- Disables write protection
- Restores each original function pointer
- Re-enables write protection
- Clears all stored function pointers
- Uses SEH for exception safety

## Service Index Configuration

### The Challenge

Service indexes (syscall numbers) are **Windows version-specific** and can change between:
- Major Windows versions (7, 8, 8.1, 10, 11)
- Minor builds and updates
- 32-bit vs 64-bit systems

### Current Implementation

The driver defines service index variables that default to 0 (lines 113-120):

```c
ULONG g_ServiceIndex_NtGdiBitBlt = 0;
ULONG g_ServiceIndex_NtGdiStretchBlt = 0;
ULONG g_ServiceIndex_NtUserGetDC = 0;
// ... etc
```

When a service index is 0, the corresponding hook is not installed.

### Configuration Methods

#### Method 1: Hardcode for Specific Windows Version

For a known Windows version, hardcode the indexes:

```c
// Windows 10 Build 19041 (20H1) - x64
ULONG g_ServiceIndex_NtGdiBitBlt = 0x1057;
ULONG g_ServiceIndex_NtGdiStretchBlt = 0x1265;
```

#### Method 2: Runtime Detection

Implement a function to detect Windows version and set indexes accordingly:

```c
VOID ConfigureServiceIndexes(VOID) {
    RTL_OSVERSIONINFOEXW versionInfo = {0};
    versionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
    
    RtlGetVersion((PRTL_OSVERSIONINFOW)&versionInfo);
    
    if (versionInfo.dwMajorVersion == 10) {
        if (versionInfo.dwBuildNumber >= 19041) {
            // Windows 10 20H1 and later
            g_ServiceIndex_NtGdiBitBlt = 0x1057;
            // ... set other indexes
        }
    }
}
```

#### Method 3: Pattern Scanning

Scan kernel memory to find function signatures and resolve indexes dynamically:

```c
ULONG FindServiceIndexByPattern(PUCHAR pattern, SIZE_T patternSize) {
    // Scan KeServiceDescriptorTable entries
    // Match function prologue patterns
    // Return found index or 0
}
```

#### Method 4: Configuration File

Load indexes from a configuration file during driver initialization:
- Store indexes in registry
- Read from INF file parameter
- Load from external config file

### Important Notes

1. **Win32k Functions**: Most GDI/User functions (NtGdi*, NtUser*) are in the **Shadow SSDT** (win32k.sys), not the regular SSDT (ntoskrnl.exe). The Shadow SSDT is process-specific and requires additional handling.

2. **DirectX Functions**: NtGdiDdDDIPresent and similar DirectX functions are in the DXGK (DirectX Graphics Kernel) subsystem and may not be in the SSDT at all.

3. **PatchGuard**: Modern Windows (Vista+) has Kernel Patch Protection (PatchGuard) that monitors the SSDT for unauthorized modifications. This driver may trigger PatchGuard on protected systems.

## Safety Features

The implementation includes comprehensive BSOD protection:

### 1. Structured Exception Handling
All SSDT manipulation code is wrapped in `__try/__except` blocks.

### 2. Parameter Validation
- NULL pointer checks on all inputs
- Service index bounds checking
- SSDT structure validation

### 3. Memory Protection Management
- Write protection is always restored, even on exceptions
- CR0 manipulation is atomic and protected

### 4. State Tracking
- Original function pointers stored before modification
- Hooks can be cleanly removed
- Prevents double-hooking

### 5. Graceful Degradation
- Driver loads even if hooks fail
- Partial hook installation is acceptable
- Detailed logging for debugging

## Determining Service Indexes

### Using WinDbg

1. Attach WinDbg to kernel
2. Find SSDT address:
   ```
   kd> dd KeServiceDescriptorTable
   ```
3. Disassemble functions:
   ```
   kd> u win32k!NtGdiBitBlt
   ```
4. Find service number in function prologue

### Using System Call Tables

Reference syscall tables for your Windows version:
- https://j00ru.vexillium.org/syscalls/nt/64/
- https://hfiref0x.github.io/NT10_syscalls.html

### Using Tools

- **ReactOS syscall tables**: Reference implementation
- **ntdiff**: Compare syscall numbers between Windows versions
- **KmdManager**: Kernel mode debugging utilities

## Testing Recommendations

### Development Environment
1. Use a Virtual Machine (VMware, VirtualBox, Hyper-V)
2. Enable kernel debugging
3. Use Debug builds with verbose logging
4. Monitor with DebugView or WinDbg

### Test Cases
1. **Basic Functionality**: Verify hooks install without BSOD
2. **Screenshot Blocking**: Test with various screenshot tools
3. **System Stability**: Run for extended periods
4. **Graceful Degradation**: Test with invalid service indexes
5. **Unload**: Verify clean driver unload and SSDT restoration

### Safety Checklist
- [ ] Test on isolated VM first
- [ ] Enable test signing or use kernel debugger
- [ ] Have WinDbg attached for debugging
- [ ] Use Debug build for detailed logging
- [ ] Create VM snapshot before testing
- [ ] Monitor system stability after loading
- [ ] Verify hooks are properly removed on unload

## Troubleshooting

### "KeServiceDescriptorTable is NULL"
- The external symbol may not be resolved
- Try declaring as `__declspec(dllimport)`
- May need to manually locate SSDT in memory

### "Service index out of range"
- Service index is incorrect for this Windows version
- Use WinDbg to verify correct indexes
- Check system call tables for your Windows build

### System Becomes Unstable
- Immediately unload driver: `driver_manager.bat stop`
- Check indexes are correct
- Verify hook functions match original signatures
- Review exception logs in DebugView

### PatchGuard BSOD (0x109)
- PatchGuard detected SSDT modification
- This is expected on protected systems
- Consider alternative hooking methods:
  - Inline hooking
  - Filter drivers
  - Callback registration

## Production Considerations

### Not Recommended For Production
SSDT hooking is generally **not recommended** for production use because:

1. **PatchGuard**: Will cause system crashes on modern Windows
2. **Compatibility**: Service indexes change between Windows versions
3. **Stability**: Direct kernel modification is inherently risky
4. **Maintenance**: Requires updates for each Windows version
5. **Security**: Can be detected and bypassed

### Alternative Approaches

For production screenshot blocking, consider:

1. **Minifilter Drivers**: File system filtering
2. **WDF Filter Drivers**: Device stack filtering  
3. **Callback Registration**: PsSetLoadImageNotifyRoutine
4. **Inline Hooking**: Function-level hooks (still risky)
5. **DRM Solutions**: Hardware-backed content protection
6. **User-Mode Hooks**: Less invasive, easier to maintain

## References

- Microsoft Windows Internals, Part 1 (7th Edition)
- Rootkit Arsenal by Bill Blunden
- Windows Kernel Programming by Pavel Yosifovich
- ReactOS Source Code: https://github.com/reactos/reactos
- NT System Call Tables: https://j00ru.vexillium.org/syscalls/

## License

This implementation is for educational and research purposes only. Use at your own risk.
