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

VTable hooking requires modifying read-only memory. The driver:

1. Changes memory protection to PAGE_READWRITE
2. Replaces the vtable entry with the hook function
3. Restores original memory protection
4. Stores the original function pointer for forwarding

```cpp
bool HookVTableMethod(void** vtable, int index, void* hookFunc, void** originalFunc) {
    DWORD oldProtection;
    VirtualProtect(&vtable[index], sizeof(void*), PAGE_READWRITE, &oldProtection);
    *originalFunc = vtable[index];
    vtable[index] = hookFunc;
    VirtualProtect(&vtable[index], sizeof(void*), oldProtection, &oldProtection);
    return true;
}
```

### Initialization Sequence

1. **DLL_PROCESS_ATTACH**: Called when DLL is loaded
2. Initialize GDI hooks using Detours or IAT hooking
3. Create temporary DirectX devices to obtain vtables
4. Hook DirectX methods by modifying vtables
5. Clean up temporary devices

### Hook Forwarding

When a hooked function is called:

1. Check global `g_BlockScreenshots` flag
2. If blocking is enabled and function should be blocked:
   - Log the attempt
   - Return failure code
3. Otherwise:
   - Forward to original function
   - Return original result

## Performance Considerations

### Overhead

- **GDI hooks**: Minimal overhead (~1-2 CPU cycles per call)
- **DirectX hooks**: Negligible impact on rendering performance
- **Memory**: ~50KB for driver code and hooks

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
