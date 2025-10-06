# SSAPT Usage Guide

## Installation

SSAPT is now a kernel-mode driver that provides system-wide screenshot protection. Installation requires Administrator privileges.

### Step 1: Enable Test Signing (Development)

For unsigned drivers during development:

```cmd
bcdedit /set testsigning on
```

**Reboot your system** after enabling test signing.

### Step 2: Install the Driver

Open Command Prompt as Administrator and run:

```cmd
driver_manager.bat install
driver_manager.bat start
```

### Step 3: Verify Installation

Check that the driver is running:

```cmd
driver_manager.bat status
```

You should see the service is "RUNNING".

## Basic Usage

The SSAPT driver is controlled via the `ssapt.exe` command-line application.

### Enable Screenshot Blocking

```cmd
ssapt.exe enable
```

This enables system-wide screenshot blocking. All screenshot attempts will be blocked.

### Disable Screenshot Blocking

```cmd
ssapt.exe disable
```

This disables the blocking. Screenshots will work normally.

### Check Status

```cmd
ssapt.exe status
```

This shows whether blocking is currently enabled or disabled.

### Help

```cmd
ssapt.exe help
```

Shows usage information and available commands.

## Testing Screenshots

### Testing the Driver

1. **Enable blocking:**
   ```cmd
   ssapt.exe enable
   ```

2. **Try taking screenshots with:**
   - Windows Snipping Tool
   - Win+PrintScreen
   - Alt+PrintScreen
   - ShareX or other third-party tools
   - OBS or other screen capture software

3. **Verify the protection:**
   - Screenshots should fail or show black screen
   - Kernel debug messages visible in DebugView
   - Event Viewer may show blocked attempts

4. **Disable and test again:**
   ```cmd
   ssapt.exe disable
   ```
   Screenshots should work normally now.

## Common Use Cases

### Protecting Sensitive Work Sessions

Enable blocking before viewing sensitive information:

```cmd
REM Enable protection
ssapt.exe enable

REM Do sensitive work (open documents, applications, etc.)
start confidential_document.pdf

REM When done, disable protection
ssapt.exe disable
```

### Automated Scripts

Integrate SSAPT into batch scripts:

```batch
@echo off
echo Starting secure session...
ssapt.exe enable

REM Run your application
start /wait sensitive_app.exe

echo Ending secure session...
ssapt.exe disable
```

### PowerShell Integration

```powershell
# Enable protection
& "ssapt.exe" enable

# Do sensitive work
Start-Process "confidential_app.exe" -Wait

# Disable protection
& "ssapt.exe" disable
```

### Scheduled Tasks

Create a scheduled task to enable protection during specific hours:

```cmd
REM Enable protection at 9 AM daily
schtasks /create /tn "Enable SSAPT" /tr "C:\path\to\ssapt.exe enable" /sc daily /st 09:00

REM Disable protection at 5 PM daily
schtasks /create /tn "Disable SSAPT" /tr "C:\path\to\ssapt.exe disable" /sc daily /st 17:00
```

## Monitoring and Debugging

### Viewing Kernel Debug Messages

Use DebugView from Sysinternals to see kernel debug output:

1. Download DebugView from Microsoft Sysinternals
2. Run as Administrator
3. Enable "Capture Kernel" from the Capture menu
4. Watch for `[SSAPT]` tagged messages

### Checking Event Viewer

1. Open Event Viewer (eventvwr.msc)
2. Navigate to Windows Logs → System
3. Look for events from "SSAPT" source

### Driver Status

Check detailed driver status:

```cmd
sc query SSAPT
```

This shows:
- Service state (RUNNING/STOPPED)
- Start type
- Service status code

## Troubleshooting

### Driver Won't Install

**Possible Causes:**
1. Not running as Administrator
2. Test signing not enabled
3. Previous installation exists

**Solutions:**
```cmd
REM Remove old installation
driver_manager.bat uninstall

REM Enable test signing
bcdedit /set testsigning on

REM Reboot
shutdown /r /t 0

REM Reinstall
driver_manager.bat install
driver_manager.bat start
```

### Control App Can't Connect

**Error:** "Failed to open SSAPT device"

**Solutions:**
1. Check driver is running:
   ```cmd
   driver_manager.bat status
   ```

2. Restart the driver:
   ```cmd
   driver_manager.bat stop
   driver_manager.bat start
   ```

3. Check for error messages in Event Viewer

### Screenshots Still Working

**Possible Causes:**
1. Blocking is disabled
2. Screenshot method not covered by kernel hooks
3. Hardware capture device

**Solutions:**
- Verify blocking is enabled: `ssapt.exe status`
- Try different screenshot tools
- Check DebugView for hook activity

### System Becomes Unstable

**If you experience system instability:**

1. **Immediately stop the driver:**
   ```cmd
   driver_manager.bat stop
   ```

2. **Check logs:**
   - DebugView kernel messages
   - Event Viewer system log
   - Windows memory dumps

3. **Uninstall if needed:**
   ```cmd
   driver_manager.bat uninstall
   ```

4. **Reboot to clean state:**
   ```cmd
   shutdown /r /t 0
   ```

## Best Practices

### 1. Enable Only When Needed

Since SSAPT now blocks system-wide, only enable it when necessary:

```batch
REM Good: Enable before sensitive work
ssapt.exe enable
start sensitive_app.exe
ssapt.exe disable
```

### 2. Check Status Before Operations

Always verify the current state:

```batch
@echo off
ssapt.exe status | findstr "ENABLED" >nul
if %errorlevel%==0 (
    echo Protection already enabled
) else (
    echo Enabling protection...
    ssapt.exe enable
)
```

### 3. Provide User Feedback

Let users know when protection is active:

```batch
@echo off
echo =====================================
echo  Entering Secure Mode
echo =====================================
ssapt.exe enable
echo Protection is now ACTIVE
pause
ssapt.exe disable
echo Protection DISABLED
```

### 4. Handle Errors Gracefully

```batch
@echo off
ssapt.exe enable
if %errorlevel% neq 0 (
    echo ERROR: Failed to enable protection
    echo Check if driver is installed and running
    exit /b 1
)
echo Protection enabled successfully
```

### 5. Clean Shutdown

Always disable before system maintenance:

```batch
REM Before updates or maintenance
ssapt.exe disable
driver_manager.bat stop
```

## Advanced Usage

### Custom Kernel Hook Behavior

To customize blocking behavior, modify `kernel_driver.c`:

```c
// Add custom logic in HookedNtGdiDdDDIPresent or other hook functions
// Recompile the kernel driver with WDK
```

### Integration with Other Applications

Create wrapper scripts for applications:

**Example: Protected Browser Session**

```batch
@echo off
REM protect_browser.bat
echo Starting protected browser session...
ssapt.exe enable
start /wait chrome.exe --incognito
ssapt.exe disable
echo Protected session ended
```

**Example: Protected Remote Desktop**

```batch
@echo off
REM protect_rdp.bat
ssapt.exe enable
mstsc.exe /v:server_address
ssapt.exe disable
```

## Command Reference

| Command | Description | Requires Admin |
|---------|-------------|----------------|
| `ssapt.exe enable` | Enable system-wide blocking | No* |
| `ssapt.exe disable` | Disable system-wide blocking | No* |
| `ssapt.exe status` | Check blocking status | No* |
| `ssapt.exe help` | Show usage information | No |
| `driver_manager.bat install` | Install kernel driver | Yes |
| `driver_manager.bat start` | Start driver service | Yes |
| `driver_manager.bat stop` | Stop driver service | Yes |
| `driver_manager.bat uninstall` | Uninstall driver | Yes |
| `driver_manager.bat status` | Check driver status | Yes |

*Requires driver to be installed and running (which requires initial admin setup)

## Uninstallation

To completely remove SSAPT:

1. **Disable blocking:**
   ```cmd
   ssapt.exe disable
   ```

2. **Stop the driver:**
   ```cmd
   driver_manager.bat stop
   ```

3. **Uninstall the driver:**
   ```cmd
   driver_manager.bat uninstall
   ```

4. **(Optional) Disable test signing:**
   ```cmd
   bcdedit /set testsigning off
   ```

5. **Reboot the system**

## Support

For issues or questions, refer to:
- **Build instructions:** BUILD.md
- **Technical documentation:** TECHNICAL.md
- **Security considerations:** SECURITY.md
- **Architecture details:** ARCHITECTURE.md
