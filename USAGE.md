# SSAPT Usage Guide

## Installation

### Method 1: Direct DLL Injection

Copy `ssapt.dll` to the same directory as your application executable.

### Method 2: System-wide Installation

1. Copy `ssapt.dll` to `C:\Windows\System32` (requires admin privileges)
2. Register the DLL if needed:
   ```bash
   regsvr32 ssapt.dll
   ```

## Integration Methods

### Static Linking

Link against the import library at compile time:

```cpp
#include "driver.h"

int main() {
    EnableBlocking();
    
    // Protected code
    MessageBox(NULL, "Screenshots are blocked", "Protected", MB_OK);
    
    DisableBlocking();
    return 0;
}
```

Compile with:
```bash
cl myapp.cpp /link ssapt.lib
```

### Dynamic Loading

Load the DLL at runtime:

```cpp
#include <windows.h>

typedef void (*EnableBlockingFunc)();
typedef void (*DisableBlockingFunc)();
typedef bool (*IsBlockingEnabledFunc)();

int main() {
    HMODULE hDll = LoadLibrary("ssapt.dll");
    if (!hDll) {
        printf("Failed to load ssapt.dll\n");
        return 1;
    }
    
    EnableBlockingFunc EnableBlocking = 
        (EnableBlockingFunc)GetProcAddress(hDll, "EnableBlocking");
    DisableBlockingFunc DisableBlocking = 
        (DisableBlockingFunc)GetProcAddress(hDll, "DisableBlocking");
    
    if (EnableBlocking) {
        EnableBlocking();
        printf("Screenshot blocking enabled\n");
    }
    
    // Protected code here
    
    if (DisableBlocking) {
        DisableBlocking();
        printf("Screenshot blocking disabled\n");
    }
    
    FreeLibrary(hDll);
    return 0;
}
```

### Injection into Running Process

Use a DLL injector to load SSAPT into a running process:

```bash
# Using Process Hacker
1. Open Process Hacker
2. Right-click on target process
3. Select "Inject DLL"
4. Choose ssapt.dll
```

## Testing Screenshots

### Built-in Test

Run the included test suite:

```bash
ssapt_test.exe
```

Expected output:
```
=== SSAPT Driver Test ===
Testing screenshot blocking functionality

>>> Testing with blocking ENABLED <<<

[TEST] Attempting GDI screenshot...
[SSAPT] Blocked BitBlt screenshot attempt
[TEST] GDI screenshot blocked successfully!

[TEST] Attempting frame buffer access via GetDIBits...
[SSAPT] Blocked GetDIBits screenshot attempt
[TEST] Frame buffer access blocked successfully!

>>> Testing with blocking DISABLED <<<

[TEST] Attempting GDI screenshot...
[TEST] GDI screenshot succeeded (blocking may not be active)

=== Test Results ===
GDI BitBlt blocking: PASS
Frame buffer blocking: PASS
GDI unblocking: PASS
Frame buffer unblocking: PASS

Overall: ALL TESTS PASSED
```

### Manual Testing

1. Enable blocking in your application
2. Try taking screenshots with:
   - Windows Snipping Tool
   - Win+PrintScreen
   - Alt+PrintScreen
   - Third-party screenshot tools

3. Verify that screenshots are:
   - Completely black
   - Failed to capture
   - Return error messages

## Common Use Cases

### Video Game Protection

```cpp
#include "driver.h"

void GameLoop() {
    EnableBlocking();
    
    while (running) {
        RenderFrame();
        ProcessInput();
        UpdateGame();
    }
    
    DisableBlocking();
}
```

### Secure Document Viewer

```cpp
#include "driver.h"

void ViewSecureDocument(const char* filename) {
    EnableBlocking();
    
    // Load and display document
    DisplayDocument(filename);
    
    // Wait for user to close
    WaitForUserClose();
    
    DisableBlocking();
}
```

### Video Conference Privacy

```cpp
#include "driver.h"

void StartVideoConference() {
    // Block screenshots during sensitive calls
    if (IsSensitiveCall()) {
        EnableBlocking();
    }
    
    RunVideoConference();
    
    DisableBlocking();
}
```

## Configuration

### Runtime Configuration

Control blocking behavior at runtime:

```cpp
// Enable/disable based on user preference
if (userSettings.blockScreenshots) {
    EnableBlocking();
}

// Check current state
if (IsBlockingEnabled()) {
    printf("Protection active\n");
}

// Toggle
if (IsBlockingEnabled()) {
    DisableBlocking();
} else {
    EnableBlocking();
}
```

### Environment Variables

Set environment variables to configure behavior:

```bash
# Windows Command Prompt
set SSAPT_DEBUG=1        # Enable debug logging
set SSAPT_AUTOSTART=1    # Auto-enable on load
```

## Troubleshooting

### Screenshots Still Working

**Possible Causes:**
1. Driver not loaded properly
2. Screenshot tool uses unsupported API
3. Blocking disabled

**Solutions:**
- Verify DLL is loaded: Check with Process Explorer
- Ensure `EnableBlocking()` was called
- Check that you have necessary permissions

### Application Crashes

**Possible Causes:**
1. Incompatible DirectX version
2. Memory protection conflicts
3. Antivirus interference

**Solutions:**
- Run with administrator privileges
- Add exception in antivirus
- Check application event logs

### Performance Issues

**Possible Causes:**
1. Debug logging enabled
2. Multiple hooks active
3. Conflicts with other overlays

**Solutions:**
- Disable debug output in release builds
- Minimize number of hooked applications
- Check for hook conflicts

## Best Practices

### 1. Enable Only When Needed

```cpp
// Bad: Always enabled
EnableBlocking();
RunApplication();

// Good: Enable for sensitive operations only
if (IsSensitiveOperation()) {
    EnableBlocking();
    PerformOperation();
    DisableBlocking();
}
```

### 2. Error Handling

```cpp
HMODULE hDll = LoadLibrary("ssapt.dll");
if (!hDll) {
    DWORD error = GetLastError();
    fprintf(stderr, "Failed to load driver: %lu\n", error);
    return 1;
}

// Use the driver...

FreeLibrary(hDll);
```

### 3. User Notification

```cpp
void EnableProtection() {
    EnableBlocking();
    ShowNotification("Screenshot protection active");
}
```

### 4. Graceful Degradation

```cpp
bool InitializeProtection() {
    HMODULE hDll = LoadLibrary("ssapt.dll");
    if (!hDll) {
        // Continue without protection
        LogWarning("Screenshot protection unavailable");
        return false;
    }
    return true;
}
```

## Advanced Usage

### Custom Hook Behavior

Modify the driver source to customize blocking behavior:

```cpp
// In driver.cpp, modify HookedBitBlt:
BOOL WINAPI HookedBitBlt(...) {
    if (g_BlockScreenshots) {
        // Custom behavior: Log to file instead of console
        LogToFile("BitBlt blocked at " + GetTimestamp());
        
        // Optional: Allow some screenshots
        if (IsWhitelistedProcess()) {
            return TrueBitBlt(...);
        }
        
        return FALSE;
    }
    return TrueBitBlt(...);
}
```

### Process Filtering

Hook only specific processes:

```cpp
bool ShouldBlockForProcess() {
    char processName[MAX_PATH];
    GetModuleFileName(NULL, processName, MAX_PATH);
    
    // Block only for certain applications
    return strstr(processName, "sensitive_app.exe") != NULL;
}
```

## API Reference Quick Guide

| Function | Description | Returns |
|----------|-------------|---------|
| `EnableBlocking()` | Activates screenshot blocking | void |
| `DisableBlocking()` | Deactivates screenshot blocking | void |
| `IsBlockingEnabled()` | Checks blocking status | bool |

## Support

For issues or questions, refer to:
- Technical documentation: TECHNICAL.md
- Build instructions: BUILD.md
- Source code comments
