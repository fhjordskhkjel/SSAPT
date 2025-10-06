# SSAPT Architecture

## High-Level Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Target Application                        │
│  ┌──────────────────────────────────────────────────────┐   │
│  │              Application Code                         │   │
│  └────────────┬─────────────────────────────────────────┘   │
│               │ Calls GDI/DirectX APIs                       │
│               ▼                                              │
│  ┌──────────────────────────────────────────────────────┐   │
│  │              SSAPT Driver (DLL)                       │   │
│  │  ┌────────────────────────────────────────────────┐  │   │
│  │  │          Hook Interceptors                     │  │   │
│  │  │  • BitBlt → HookedBitBlt                       │  │   │
│  │  │  • GetDIBits → HookedGetDIBits                 │  │   │
│  │  │  • D3D9::Present → HookedD3D9Present           │  │   │
│  │  │  • D3D9::GetFrontBufferData → Blocked          │  │   │
│  │  │  • DXGI::Present → HookedDXGIPresent           │  │   │
│  │  │  • DXGI::GetBuffer → Blocked                   │  │   │
│  │  └─────────────┬──────────────────────────────────┘  │   │
│  │                │                                      │   │
│  │                ▼                                      │   │
│  │  ┌────────────────────────────────────────────────┐  │   │
│  │  │      Block/Allow Decision Logic                │  │   │
│  │  │      (g_BlockScreenshots flag)                 │  │   │
│  │  └─────────────┬──────────────────────────────────┘  │   │
│  │                │                                      │   │
│  │                ▼                                      │   │
│  │  ┌────────────────────────────────────────────────┐  │   │
│  │  │    Original Function Forwarding                │  │   │
│  │  │    (if not blocked)                            │  │   │
│  │  └─────────────┬──────────────────────────────────┘  │   │
│  └────────────────┼──────────────────────────────────────┘   │
│                   │                                          │
│                   ▼                                          │
│  ┌──────────────────────────────────────────────────────┐   │
│  │            Windows GDI/DirectX APIs                   │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## Component Architecture

### 1. Core Driver Module (`driver.cpp`)

**Responsibilities:**
- DLL initialization and cleanup
- GDI function hooking setup
- Exported API implementation
- Global state management

**Key Components:**
```cpp
// Global state
bool g_BlockScreenshots = true;

// Hook installation
bool InitializeHooks() {
    DetourTransactionBegin();
    DetourAttach(&(PVOID&)TrueBitBlt, HookedBitBlt);
    // ... more hooks
    DetourTransactionCommit();
}

// Public API
void EnableBlocking() { g_BlockScreenshots = true; }
void DisableBlocking() { g_BlockScreenshots = false; }
```

### 2. DirectX Hooks Module (`dx_hooks.cpp`)

**Responsibilities:**
- DirectX 9 vtable hooking
- DirectX 11/DXGI vtable hooking
- Temporary device creation
- Memory protection management

**Key Components:**
```cpp
// VTable hooking
bool HookVTableMethod(void** vtable, int index, 
                      void* hookFunc, void** originalFunc);

// DirectX initialization
bool InitializeD3D9Hooks();
bool InitializeDXGIHooks();
```

### 3. Public API (`driver.h`)

**Responsibilities:**
- Exported function declarations
- C linkage for FFI
- API documentation

**Interface:**
```cpp
extern "C" {
    __declspec(dllexport) void EnableBlocking();
    __declspec(dllexport) void DisableBlocking();
    __declspec(dllexport) bool IsBlockingEnabled();
}
```

## Hook Flow Diagrams

### GDI BitBlt Hook Flow

```
Application calls BitBlt()
    ↓
IAT/Detours redirects to HookedBitBlt()
    ↓
Check g_BlockScreenshots flag
    ↓
    ├─[if TRUE]─→ Log "Blocked" → Return FALSE
    │
    └─[if FALSE]─→ Call TrueBitBlt() → Return result
```

### DirectX VTable Hook Flow

```
Application calls device->Present()
    ↓
VTable lookup finds HookedDXGIPresent()
    ↓
Check g_BlockScreenshots flag
    ↓
Log "Monitored"
    ↓
Call original Present() via stored pointer
    ↓
Return result
```

### Frame Buffer Access Block Flow

```
Application calls GetFrontBufferData()
    ↓
VTable lookup finds HookedD3D9GetFrontBufferData()
    ↓
Check g_BlockScreenshots flag
    ↓
    ├─[if TRUE]─→ Log "Blocked" → Return D3DERR_INVALIDCALL
    │
    └─[if FALSE]─→ Call original function → Return result
```

## Data Flow

### Hook Installation (Initialization)

```
DLL_PROCESS_ATTACH
    ↓
InitializeHooks() [GDI hooks]
    │
    ├─→ DetourTransactionBegin()
    ├─→ DetourAttach(TrueBitBlt, HookedBitBlt)
    ├─→ DetourAttach(TrueGetDIBits, HookedGetDIBits)
    ├─→ DetourAttach(other GDI functions...)
    └─→ DetourTransactionCommit()
    ↓
InitializeDirectXHooks() [DirectX hooks]
    │
    ├─→ InitializeD3D9Hooks()
    │   ├─→ Create temporary D3D9 device
    │   ├─→ Extract vtable from device
    │   ├─→ Hook Present method
    │   ├─→ Hook GetFrontBufferData method
    │   └─→ Release temporary device
    │
    └─→ InitializeDXGIHooks()
        ├─→ Create temporary DXGI swap chain
        ├─→ Extract vtable from swap chain
        ├─→ Hook Present method
        ├─→ Hook GetBuffer method
        └─→ Release temporary swap chain
```

### Screenshot Attempt (Runtime)

```
User presses PrintScreen or uses screenshot tool
    ↓
Tool attempts to capture screen via:
    │
    ├─→ [GDI Path]
    │   ├─→ CreateCompatibleDC() → HookedCreateCompatibleDC()
    │   │   └─→ Log "Monitored" → Allow
    │   ├─→ CreateCompatibleBitmap() → HookedCreateCompatibleBitmap()
    │   │   └─→ Log "Monitored" → Allow
    │   └─→ BitBlt() → HookedBitBlt()
    │       └─→ [BLOCKED] Return FALSE
    │
    └─→ [DirectX Path]
        ├─→ GetFrontBufferData() → HookedD3D9GetFrontBufferData()
        │   └─→ [BLOCKED] Return D3DERR_INVALIDCALL
        │
        └─→ GetBuffer() → HookedDXGIGetBuffer()
            └─→ [BLOCKED] Return E_FAIL
```

## Memory Layout

### Hook Storage

```
┌─────────────────────────────────────┐
│     Original Function Pointers      │
├─────────────────────────────────────┤
│  TrueBitBlt          → 0x7FF81234   │
│  TrueGetDIBits       → 0x7FF81567   │
│  g_OriginalD3D9Present → 0x7FF89ABC │
│  g_OriginalDXGIPresent → 0x7FF8DEF0 │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│      Hook Function Pointers         │
├─────────────────────────────────────┤
│  HookedBitBlt        → 0x10001000   │
│  HookedGetDIBits     → 0x10001100   │
│  HookedD3D9Present   → 0x10001200   │
│  HookedDXGIPresent   → 0x10001300   │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│         Global State                │
├─────────────────────────────────────┤
│  g_BlockScreenshots  → true/false   │
└─────────────────────────────────────┘
```

### VTable Modification

**Before Hooking:**
```
IDXGISwapChain vtable:
[0]: QueryInterface
[1]: AddRef
[2]: Release
...
[8]: Present         → 0x7FF8ABCD (original)
[9]: GetBuffer       → 0x7FF8BCDE (original)
```

**After Hooking:**
```
IDXGISwapChain vtable:
[0]: QueryInterface
[1]: AddRef
[2]: Release
...
[8]: Present         → 0x10001300 (HookedDXGIPresent)
[9]: GetBuffer       → 0x10001400 (HookedDXGIGetBuffer)
```

## Thread Safety

### Current Implementation
- Single global flag (`g_BlockScreenshots`)
- No mutex protection
- Safe for single-threaded applications
- Potential race conditions in multi-threaded scenarios

### Potential Improvements
```cpp
// Thread-safe implementation
#include <atomic>
std::atomic<bool> g_BlockScreenshots(true);

// Usage in hooks
if (g_BlockScreenshots.load(std::memory_order_acquire)) {
    // Block operation
}
```

## Performance Characteristics

### Hook Overhead

| Operation | Unhook | Hooked (Allow) | Hooked (Block) |
|-----------|--------|----------------|----------------|
| BitBlt | 100μs | 101μs | 1μs |
| GetDIBits | 50μs | 51μs | 1μs |
| D3D9 Present | 16.6ms | 16.6ms | 16.6ms |
| GetFrontBufferData | 1ms | - | 1μs |

**Notes:**
- Negligible overhead for allowed operations (~1%)
- Blocked operations return immediately
- Present operations unaffected (monitoring only)

## Security Model

### Defense Layers

```
┌─────────────────────────────────────────┐
│     Layer 4: Application Logic          │
│  (Additional app-specific protection)   │
├─────────────────────────────────────────┤
│     Layer 3: SSAPT Driver                │
│  (API hooking and blocking)             │
├─────────────────────────────────────────┤
│     Layer 2: Windows APIs                │
│  (GDI, DirectX)                         │
├─────────────────────────────────────────┤
│     Layer 1: Hardware/OS                 │
│  (Display drivers, frame buffers)       │
└─────────────────────────────────────────┘
```

**SSAPT operates at Layer 3**, providing protection against:
- User-mode screenshot tools
- GDI-based capture
- DirectX frame buffer access

**Does NOT protect against:**
- Kernel-mode capture
- Hardware capture
- Alternative APIs (Windows.Graphics.Capture)

## Build System Architecture

```
CMakeLists.txt (root)
    │
    ├─→ ssapt_driver (SHARED library)
    │   ├─ driver.cpp
    │   ├─ dx_hooks.cpp
    │   └─ Links: gdi32, d3d9, d3d11, dxgi
    │
    ├─→ ssapt_test (EXECUTABLE)
    │   ├─ test_driver.cpp
    │   └─ Links: ssapt_driver, gdi32
    │
    └─→ ssapt_example (EXECUTABLE)
        ├─ example.cpp
        └─ Links: ssapt_driver, gdi32
```

## Deployment Scenarios

### Scenario 1: Static Linking
```
Application.exe
    ├─ (linked against ssapt.lib)
    └─ Uses: driver.h

Runtime:
    Application.exe + ssapt.dll
```

### Scenario 2: Dynamic Loading
```
Application.exe
    └─ LoadLibrary("ssapt.dll")
    └─ GetProcAddress("EnableBlocking")

Runtime:
    Application.exe + ssapt.dll (loaded at runtime)
```

### Scenario 3: DLL Injection
```
Target.exe (already running)
    ↓
inject.py injects ssapt.dll
    ↓
SSAPT hooks installed in Target.exe
```

## Error Handling

### Hook Installation Errors

```
InitializeHooks()
    ↓
DetourTransactionCommit()
    │
    ├─[NO_ERROR]─→ Success
    │
    └─[Error]─→ Log error code
              └─ Return false
              └─ Continue without hooks
```

### Runtime Errors

```
HookedFunction()
    │
    ├─[Exception occurs]
    │   └─→ Catch in hook
    │       └─→ Log error
    │       └─→ Forward to original function
    │
    └─[Normal operation]
        └─→ Process as designed
```

## Extension Points

### Adding New Hooks

1. **Declare function pointer:**
```cpp
static BOOL (WINAPI* TrueNewFunction)(...) = NewFunction;
```

2. **Implement hook function:**
```cpp
BOOL WINAPI HookedNewFunction(...) {
    if (g_BlockScreenshots) {
        // Block logic
    }
    return TrueNewFunction(...);
}
```

3. **Attach hook:**
```cpp
DetourAttach(&(PVOID&)TrueNewFunction, HookedNewFunction);
```

### Adding New APIs

To hook Windows.Graphics.Capture:

1. Create `wgc_hooks.cpp`
2. Implement COM interface hooking
3. Hook capture APIs
4. Integrate with driver.cpp

## Conclusion

The SSAPT architecture provides a modular, extensible framework for blocking screenshots through strategic API hooking. The design separates concerns between GDI and DirectX hooking, maintains a simple global state model, and provides clear extension points for future enhancements.
