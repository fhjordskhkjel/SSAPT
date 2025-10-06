# SSAPT Architecture

## High-Level Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      User Mode                               │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐   │
│  │      Control Application (ssapt.exe)                  │   │
│  │                                                       │   │
│  │  Commands: enable, disable, status                   │   │
│  └─────────────┬────────────────────────────────────────┘   │
│                │                                             │
│                │ IOCTL Commands                              │
│                │ (CreateFile, DeviceIoControl)               │
└────────────────┼─────────────────────────────────────────────┘
                 │
════════════════ │ ═══════════════════════════════════════════
                 │ Kernel/User Mode Boundary
════════════════ ▼ ═══════════════════════════════════════════
┌─────────────────────────────────────────────────────────────┐
│                     Kernel Mode                              │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐   │
│  │         SSAPT Kernel Driver (ssapt.sys)              │   │
│  │                                                       │   │
│  │  ┌──────────────────────────────────────────────┐   │   │
│  │  │   Device Object (\Device\SSAPT)              │   │   │
│  │  │   • IRP_MJ_CREATE                            │   │   │
│  │  │   • IRP_MJ_CLOSE                             │   │   │
│  │  │   • IRP_MJ_DEVICE_CONTROL (IOCTLs)           │   │   │
│  │  └─────────────┬────────────────────────────────┘   │   │
│  │                │                                     │   │
│  │                ▼                                     │   │
│  │  ┌──────────────────────────────────────────────┐   │   │
│  │  │   Global State (Spin Lock Protected)         │   │   │
│  │  │   • BlockingEnabled flag                     │   │   │
│  │  └─────────────┬────────────────────────────────┘   │   │
│  │                │                                     │   │
│  │                ▼                                     │   │
│  │  ┌──────────────────────────────────────────────┐   │   │
│  │  │   Kernel Hook Interceptors (10 hooks)        │   │   │
│  │  │   • NtGdiDdDDIPresent → Monitored            │   │   │
│  │  │   • NtGdiDdDDIGetDisplayModeList → Blocked   │   │   │
│  │  │   • NtGdiBitBlt → Blocked (large ops)        │   │   │
│  │  │   • NtGdiStretchBlt → Blocked (large ops)    │   │   │
│  │  │   • NtUserGetDC → Monitored                  │   │   │
│  │  │   • NtUserGetWindowDC → Monitored            │   │   │
│  │  │   • NtGdiGetDIBitsInternal → Blocked (reads) │   │   │
│  │  │   • NtGdiCreateCompatibleDC → Monitored      │   │   │
│  │  │   • NtGdiCreateCompatibleBitmap → Monitored  │   │   │
│  │  │   • NtUserPrintWindow → Blocked              │   │   │
│  │  └─────────────┬────────────────────────────────┘   │   │
│  └────────────────┼──────────────────────────────────────┘   │
│                   │                                          │
│                   ▼                                          │
│  ┌──────────────────────────────────────────────────────┐   │
│  │      Windows Kernel Graphics Subsystem               │   │
│  │      (win32k.sys, dxgkrnl.sys)                       │   │
│  └──────────────────────────────────────────────────────┘   │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

## Component Architecture

### 1. Kernel Driver Module (`kernel_driver.c`)

**Responsibilities:**
- Kernel driver initialization and cleanup
- Device and symbolic link creation
- IOCTL request handling
- Kernel graphics API hooking
- Thread-safe state management

**Key Components:**
```c
// Global state (spin lock protected)
typedef struct _SSAPT_GLOBALS {
    PDEVICE_OBJECT DeviceObject;
    BOOLEAN BlockingEnabled;
    KSPIN_LOCK StateLock;
} SSAPT_GLOBALS;

// Driver entry point
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);

// IOCTL handler
NTSTATUS DeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);

// Hook functions
NTSTATUS HookedNtGdiDdDDIPresent(VOID* pPresentData);
```

### 2. Control Application (`control_app.cpp`)

**Responsibilities:**
- User-mode interface to kernel driver
- IOCTL command execution
- Status reporting
- Command-line argument parsing

**Key Components:**
```cpp
class SSAPTController {
    HANDLE hDevice;
    bool Enable();
    bool Disable();
    bool GetStatus(bool* pEnabled);
    void ShowStatus();
};

// Command handlers
int main(int argc, char* argv[]);
```

### 3. Driver Management (`driver_manager.bat`)

**Responsibilities:**
- Driver service installation
- Service start/stop control
- Service status query
- Service uninstallation

**Commands:**
```batch
driver_manager.bat install
driver_manager.bat start
driver_manager.bat stop
driver_manager.bat uninstall
driver_manager.bat status
```

## Communication Flow Diagrams

### Enable/Disable Flow

```
User executes: ssapt.exe enable
    ↓
Control app opens device: \\.\SSAPT
    ↓
Send IOCTL_SSAPT_ENABLE via DeviceIoControl()
    ↓
    [Kernel Mode]
    ↓
Kernel driver receives IRP_MJ_DEVICE_CONTROL
    ↓
Acquire spin lock
    ↓
Set BlockingEnabled = TRUE
    ↓
Release spin lock
    ↓
Complete IRP with STATUS_SUCCESS
    ↓
    [User Mode]
    ↓
Control app receives success
    ↓
Display: "Screenshot blocking ENABLED"
```

### Status Query Flow

```
User executes: ssapt.exe status
    ↓
Control app opens device: \\.\SSAPT
    ↓
Send IOCTL_SSAPT_STATUS via DeviceIoControl()
    ↓
    [Kernel Mode]
    ↓
Acquire spin lock (read)
    ↓
Read BlockingEnabled flag
    ↓
Release spin lock
    ↓
Write status to output buffer
    ↓
Complete IRP with STATUS_SUCCESS
    ↓
    [User Mode]
    ↓
Control app reads status from buffer
    ↓
Display: "Status: ENABLED/DISABLED"
```

### Kernel Hook Flow

```
Application calls graphics function (e.g., BitBlt)
    ↓
Windows dispatches to win32k.sys
    ↓
SSAPT hook intercepts (e.g., HookedNtGdiBitBlt)
    ↓
Validate parameters (NULL checks)
    ↓
Acquire spin lock (read)
    ↓
Check BlockingEnabled flag
    ↓
Release spin lock
    ↓
    ├─[if TRUE and large op]─→ Log "Blocked" → Return FALSE
    │
    └─[if FALSE or small op]─→ Call original → Return result

Note: 10 hooks now in place:
  - NtGdiDdDDIPresent (monitoring)
  - NtGdiDdDDIGetDisplayModeList (blocking)
  - NtGdiBitBlt (blocking large transfers >100px)
  - NtGdiStretchBlt (blocking large transfers >100px)
  - NtUserGetDC (monitoring)
  - NtUserGetWindowDC (monitoring)
  - NtGdiGetDIBitsInternal (blocking pixel reads)
  - NtGdiCreateCompatibleDC (monitoring)
  - NtGdiCreateCompatibleBitmap (monitoring)
  - NtUserPrintWindow (blocking)
```

## Data Flow

### Driver Installation and Startup

```
Administrator runs: driver_manager.bat install
    ↓
sc create SSAPT binPath=ssapt.sys type=kernel
    ↓
Service created in registry
    ↓
Administrator runs: driver_manager.bat start
    ↓
Service Control Manager loads ssapt.sys
    ↓
    [Kernel Mode]
    ↓
DriverEntry() called
    │
    ├─→ Initialize global state
    ├─→ Create device object: \Device\SSAPT
    ├─→ Create symbolic link: \??\SSAPT
    ├─→ Set up IRP handlers (CREATE, CLOSE, DEVICE_CONTROL)
    ├─→ Initialize kernel hooks (SSDT/inline)
    └─→ Return STATUS_SUCCESS
    ↓
Driver loaded and ready
```

### Screenshot Blocking (Runtime)

```
Any application attempts graphics operation
    ↓
Windows kernel dispatches to graphics subsystem
    ↓
SSAPT hook intercepts kernel call
    │
    ├─→ [If NtGdiDdDDIPresent]
    │   ├─→ Acquire spin lock
    │   ├─→ Read BlockingEnabled
    │   ├─→ Release spin lock
    │   ├─→ Log if enabled
    │   └─→ Forward to original function
    │
    └─→ [If NtGdiDdDDIGetDisplayModeList]
        ├─→ Acquire spin lock
        ├─→ Check BlockingEnabled
        └─→ If TRUE:
            │   └─→ Return STATUS_ACCESS_DENIED
            └─→ If FALSE:
                └─→ Forward to original function
```

## Memory Layout

### Kernel Driver State

```
┌─────────────────────────────────────┐
│       SSAPT_GLOBALS Structure       │
├─────────────────────────────────────┤
│  DeviceObject      → PDEVICE_OBJECT │
│  BlockingEnabled   → BOOLEAN        │
│  StateLock         → KSPIN_LOCK     │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│     Original Function Pointers      │
│      (Kernel Address Space)         │
├─────────────────────────────────────┤
│  g_OriginalNtGdiDdDDIPresent        │
│     → 0xFFFFF80001234000 (kernel)   │
│  g_OriginalNtGdiDdDDIGetDisplayMode │
│     → 0xFFFFF80001234100 (kernel)   │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│       Hook Function Pointers        │
│      (Driver Code Section)          │
├─────────────────────────────────────┤
│  HookedNtGdiDdDDIPresent            │
│     → 0xFFFFF88001000000 (driver)   │
│  HookedNtGdiDdDDIGetDisplayModeList │
│     → 0xFFFFF88001000100 (driver)   │
└─────────────────────────────────────┘
```

### Device Object Structure

```
Device Object: \Device\SSAPT
    ↓
Symbolic Link: \??\SSAPT
    ↓
User-mode access: \\.\SSAPT
    ↓
IRP Handlers:
    • IRP_MJ_CREATE
    • IRP_MJ_CLOSE
    • IRP_MJ_DEVICE_CONTROL
```

## Thread Safety

### Kernel Implementation
- Global state protected by **KSPIN_LOCK**
- All state access wrapped in `KeAcquireSpinLock/KeReleaseSpinLock`
- Safe for multi-processor systems
- Proper IRQL level management

### Spin Lock Usage
```c
// Reading state
KIRQL oldIrql;
BOOLEAN shouldBlock;

KeAcquireSpinLock(&g_Globals.StateLock, &oldIrql);
shouldBlock = g_Globals.BlockingEnabled;
KeReleaseSpinLock(&g_Globals.StateLock, oldIrql);

// Modifying state (in IOCTL handler)
KeAcquireSpinLock(&g_Globals.StateLock, &oldIrql);
g_Globals.BlockingEnabled = TRUE;  // or FALSE
KeReleaseSpinLock(&g_Globals.StateLock, oldIrql);
```

### IRQL Considerations
- Spin locks raise IRQL to DISPATCH_LEVEL
- Hook functions execute at various IRQLs
- Proper IRQL management prevents system deadlocks

## Performance Characteristics

### Kernel Hook Overhead

| Operation | Unhooked | Hooked (Monitor) | Hooked (Block) |
|-----------|----------|------------------|----------------|
| NtGdiDdDDIPresent | Direct call | +Spin lock +Check | +Spin lock +Check |
| Graphics operation | ~16ms | ~16.001ms | ~0.001ms (blocked) |
| IOCTL command | N/A | ~0.1ms | ~0.1ms |

**Notes:**
- Spin lock overhead: ~1-5μs
- Negligible impact on frame rates
- Blocked operations return immediately
- System-wide protection (no per-process overhead)

## Security Model

### Defense Layers

```
┌─────────────────────────────────────────┐
│     Layer 4: Application Logic          │
│  (Additional app-specific protection)   │
├─────────────────────────────────────────┤
│     Layer 3: User-Mode APIs              │
│  (GDI, DirectX, Windows.Graphics)       │
├─────────────────────────────────────────┤
│     Layer 2: SSAPT Kernel Driver         │
│  (Kernel-mode hooking and blocking)     │  ← We operate here
├─────────────────────────────────────────┤
│     Layer 1: Windows Kernel              │
│  (win32k.sys, dxgkrnl.sys)              │
├─────────────────────────────────────────┤
│     Layer 0: Hardware                    │
│  (GPU, display hardware)                │
└─────────────────────────────────────────┘
```

**SSAPT operates at Layer 2 (Kernel Mode)**, providing protection against:
- User-mode screenshot tools (all processes)
- System-wide screenshot blocking
- Kernel graphics API interception
- Most screenshot capture methods

**Provides stronger protection than:**
- User-mode DLL injection
- Per-process hooking
- Application-level protection

**Does NOT protect against:**
- Physical camera capture
- Hardware capture cards
- Direct GPU memory access (DMA)
- Other kernel drivers operating at same or lower level

## Build System Architecture

```
Build System (Two-Part)
    │
    ├─→ User-Mode (CMake)
    │   │
    │   └─→ ssapt_control (EXECUTABLE)
    │       ├─ control_app.cpp
    │       └─ Output: ssapt.exe
    │
    └─→ Kernel-Mode (WDK)
        │
        ├─→ ssapt_driver (KERNEL DRIVER)
        │   ├─ kernel_driver.c
        │   ├─ sources
        │   ├─ ssapt.inf
        │   └─ Output: ssapt.sys
        │
        └─→ Build Requirements:
            ├─ Windows Driver Kit (WDK)
            ├─ Visual Studio with WDK integration
            └─ Code signing certificate (for production)
```

## Deployment Scenarios

### Scenario 1: Development/Testing
```
1. Enable test signing:
   bcdedit /set testsigning on
   (reboot)

2. Install driver:
   driver_manager.bat install
   driver_manager.bat start

3. Use control app:
   ssapt.exe enable/disable/status
```

### Scenario 2: Production Deployment
```
1. Sign driver with valid certificate
2. Submit to Microsoft for attestation
3. Create installation package (MSI)
4. Install via:
   - MSI installer
   - INF file with pnputil
   - Service Control Manager (sc)

5. Distribute control app
```

### Scenario 3: Enterprise Deployment
```
1. Use Group Policy to deploy:
   - Driver installation
   - Default blocking state
   - Auto-start configuration

2. Centralized management:
   - Remote enable/disable via scripts
   - Scheduled tasks
   - Configuration management tools
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
