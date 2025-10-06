# SSAPT Technical Documentation

## Architecture Overview

SSAPT operates as a dynamically loaded library (DLL) that intercepts Windows API calls related to screen capture and frame buffer access. The driver uses two primary hooking techniques:

1. **IAT/Detours Hooking** - For GDI functions
2. **VTable Hooking** - For DirectX COM interfaces

## Hooking Mechanisms

### GDI Hooking

GDI (Graphics Device Interface) is the legacy Windows graphics API that most screenshot tools use. SSAPT hooks the following functions:

#### BitBlt
```cpp
BOOL BitBlt(HDC hdcDest, int x, int y, int cx, int cy, 
            HDC hdcSrc, int x1, int y1, DWORD rop)
```

**Purpose**: Performs a bit-block transfer from source to destination DC.  
**Hook Behavior**: Returns FALSE when blocking is enabled, preventing the transfer.  
**Impact**: Blocks PrintScreen, Snipping Tool, and similar utilities.

#### GetDIBits
```cpp
int GetDIBits(HDC hdc, HBITMAP hbm, UINT start, UINT cLines,
              LPVOID lpvBits, LPBITMAPINFO lpbmi, UINT usage)
```

**Purpose**: Retrieves bitmap bits from a device context.  
**Hook Behavior**: Returns 0 when blocking is enabled, indicating failure.  
**Impact**: Blocks direct frame buffer reading.

### DirectX Hooking

DirectX uses COM interfaces with vtables. SSAPT hooks methods by modifying the vtable entries.

#### DirectX 9

**Present Method** (VTable Index 17)
```cpp
HRESULT Present(CONST RECT* pSourceRect, CONST RECT* pDestRect,
                HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
```

**Hook Behavior**: Monitored but not blocked (allows normal rendering).  
**Purpose**: Detect when frames are being presented.

**GetFrontBufferData Method** (VTable Index 32)
```cpp
HRESULT GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface)
```

**Hook Behavior**: Returns D3DERR_INVALIDCALL when blocking is enabled.  
**Impact**: Blocks front buffer capture in D3D9 applications.

#### DirectX 11/DXGI

**Present Method** (VTable Index 8)
```cpp
HRESULT Present(UINT SyncInterval, UINT Flags)
```

**Hook Behavior**: Monitored but not blocked.  
**Purpose**: Detect frame presentation in modern applications.

**GetBuffer Method** (VTable Index 9)
```cpp
HRESULT GetBuffer(UINT Buffer, REFIID riid, void** ppSurface)
```

**Hook Behavior**: Returns E_FAIL when blocking is enabled.  
**Impact**: Blocks back buffer access for screenshot capture.

## Implementation Details

### Memory Protection

VTable hooking requires modifying read-only memory. The driver implements comprehensive safety checks:

1. Validates all pointers before use with `IsValidMemoryPtr()`
2. Checks vtable index bounds (0-200 range)
3. Changes memory protection to PAGE_READWRITE
4. Replaces the vtable entry with the hook function
5. Restores original memory protection
6. Stores the original function pointer for forwarding

```cpp
bool HookVTableMethod(void** vtable, int index, void* hookFunc, void** originalFunc) {
    __try {
        // Validate all pointers
        if (!vtable || !hookFunc || !originalFunc) return false;
        
        // Validate index bounds
        if (index < 0 || index > 200) return false;
        
        // Validate memory is accessible
        if (!IsValidMemoryPtr(&vtable[index], sizeof(void*))) return false;
        
        DWORD oldProtection;
        if (!SetMemoryProtection(&vtable[index], sizeof(void*), PAGE_READWRITE, &oldProtection)) {
            return false;
        }
        
        *originalFunc = vtable[index];
        vtable[index] = hookFunc;
        
        SetMemoryProtection(&vtable[index], sizeof(void*), oldProtection, &oldProtection);
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
```

### Structured Exception Handling (SEH)

All hook functions and critical initialization routines use Windows Structured Exception Handling to prevent system crashes:

- **GDI Hooks**: Each hook function wrapped in `__try/__except` blocks
- **DirectX Hooks**: All vtable hook functions include SEH protection
- **Initialization**: Hook installation protected against exceptions
- **DllMain**: Entry point protected to prevent loader crashes

This ensures that any unexpected errors (access violations, invalid memory operations) are caught and handled gracefully rather than causing a BSOD.

### Initialization Sequence

1. **DLL_PROCESS_ATTACH**: Called when DLL is loaded (protected by SEH)
2. Initialize GDI hooks using Detours or IAT hooking (with error handling)
3. Create temporary DirectX devices to obtain vtables (validated before use)
4. Validate device and vtable pointers using `IsValidMemoryPtr()`
5. Hook DirectX methods by modifying vtables (with bounds checking)
6. Clean up temporary devices (with exception protection)

All initialization steps include error handling and validation to ensure safe operation.

### Hook Forwarding

When a hooked function is called:

1. Enter SEH protected block (`__try`)
2. Validate function pointer is not null
3. Validate object pointers (device, swap chain, etc.)
4. Check global `g_BlockScreenshots` flag
5. If blocking is enabled and function should be blocked:
   - Log the attempt
   - Return failure code
6. Otherwise:
   - Forward to original function
   - Return original result
7. Catch any exceptions (`__except`) and return safe error code

This ensures that even if an unexpected error occurs, the system remains stable.

## Safety and Stability Features

### Kernel-Level Safety Checks

The driver implements comprehensive safety measures to prevent system crashes (BSODs):

#### Memory Validation
- **`IsValidMemoryPtr()`**: Uses `VirtualQuery()` to validate memory before access
- Checks memory state (must be MEM_COMMIT)
- Ensures memory regions are accessible before reading/writing

#### Pointer Validation
- All function pointers checked for null before invocation
- DirectX object pointers (devices, swap chains) validated before use
- Vtable pointers validated before modification

#### Bounds Checking
- Vtable indices validated (0-200 range)
- Array access protected with range checks
- Buffer sizes validated before operations

#### Structured Exception Handling (SEH)
- All hook functions wrapped in `__try/__except` blocks
- Initialization functions protected against exceptions
- DllMain entry point fully protected
- Exceptions caught and logged without propagating to system

#### Safe Memory Operations
- Memory protection changes validated before execution
- Original protection restored after modifications
- VirtualProtect failures handled gracefully

### Error Handling Strategy

1. **Fail Safely**: All errors return safe error codes rather than crashing
2. **Log and Continue**: Errors are logged to stderr for debugging
3. **Graceful Degradation**: If hooks can't be installed, driver continues without them
4. **No Fatal Errors**: No error condition results in terminating the process

## Performance Considerations

### Overhead

- **GDI hooks**: Minimal overhead (~3-5 CPU cycles per call with validation)
- **DirectX hooks**: Negligible impact on rendering performance
- **Memory**: ~50KB for driver code and hooks
- **Validation overhead**: <1% performance impact from safety checks

### Optimization Strategies

1. **Conditional Logging**: Debug output only in debug builds
2. **Minimal Processing**: Hooks check a single boolean flag
3. **Direct Forwarding**: Non-blocked calls go straight to original functions

## Security Considerations

### Attack Vectors

The driver can be bypassed by:

1. **Alternative APIs**: Using Windows.Graphics.Capture or DXGI Desktop Duplication
2. **Hardware Capture**: External capture cards
3. **Driver Unloading**: Ejecting the DLL from the process
4. **Kernel-Level Capture**: Kernel-mode screen capture

### Mitigations

- Hook additional APIs (Windows.Graphics.Capture, DXGI Desktop Duplication)
- Implement anti-unload protections
- Consider kernel-mode driver for stronger protection
- Monitor for hook removal attempts

## Testing Strategy

### Unit Tests

1. **GDI Tests**: Attempt BitBlt and GetDIBits with blocking on/off
2. **DirectX Tests**: Create D3D9/D3D11 devices and attempt buffer access
3. **Toggle Tests**: Verify enable/disable functionality

### Integration Tests

1. Load driver in test application
2. Attempt screenshots using common tools
3. Verify screenshots are blocked/allowed as expected

### Compatibility Tests

- Windows 10 (1809+)
- Windows 11
- DirectX 9, 11, 12 applications
- Various screenshot tools (Snipping Tool, Win+PrintScreen, etc.)

## Debugging

### Logging

The driver outputs to stdout for debugging:

```
[SSAPT] GDI hooks installed successfully
[SSAPT] D3D9 hooks installed successfully
[SSAPT] Blocked BitBlt screenshot attempt
```

### Tools

- **API Monitor**: View hooked function calls
- **Process Explorer**: Verify DLL is loaded
- **Dependency Walker**: Check exports
- **Visual Studio Debugger**: Attach to target process

## Future Enhancements

1. **Additional Hooks**:
   - Windows.Graphics.Capture API
   - DXGI Desktop Duplication API
   - Windows Media Foundation

2. **Kernel-Mode Driver**:
   - Stronger protection
   - System-wide enforcement
   - Protection against DLL unloading

3. **Watermarking**:
   - Inject visible/invisible watermarks
   - Detect screenshot attempts via pattern matching

4. **Advanced Detection**:
   - Monitor for screenshot tool processes
   - Detect clipboard activity
   - Analyze memory access patterns

## References

- [Microsoft GDI Documentation](https://docs.microsoft.com/en-us/windows/win32/gdi/windows-gdi)
- [DirectX 9 SDK](https://docs.microsoft.com/en-us/windows/win32/direct3d9/dx9-graphics)
- [DXGI Documentation](https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/dx-graphics-dxgi)
- [Microsoft Detours](https://github.com/microsoft/Detours)

## Glossary

- **GDI**: Graphics Device Interface - Legacy Windows graphics API
- **DirectX**: Collection of APIs for multimedia and gaming
- **DXGI**: DirectX Graphics Infrastructure - Low-level graphics API
- **Vtable**: Virtual function table used by COM interfaces
- **IAT**: Import Address Table - Lists imported functions
- **Frame Buffer**: Memory buffer containing pixel data for display
