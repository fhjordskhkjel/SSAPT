# SSAPT Quick Start Guide

## What is SSAPT?

SSAPT is a **Windows kernel-mode driver** that provides **system-wide screenshot blocking**. Unlike user-mode solutions, it works at the kernel level to block screenshots across all applications.

## Quick Installation (Development/Testing)

### Prerequisites
- Windows 10/11 (64-bit)
- Administrator privileges
- Windows Driver Kit (WDK) installed

### Step 1: Enable Test Signing

Open Command Prompt as Administrator:

```cmd
bcdedit /set testsigning on
```

**Reboot your computer** - This is required!

### Step 2: Build the Components

**Control Application (User-Mode):**
```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

**Kernel Driver (Requires WDK):**
- Open Visual Studio with WDK
- Create new Kernel Mode Driver project
- Add `kernel_driver.c` to project
- Build as x64 Release

### Step 3: Install the Driver

Copy `ssapt.sys` to the SSAPT directory, then:

```cmd
driver_manager.bat install
driver_manager.bat start
```

### Step 4: Verify Installation

```cmd
driver_manager.bat status
```

You should see: `STATE: 4  RUNNING`

## Basic Usage

### Enable Screenshot Blocking

```cmd
ssapt.exe enable
```

### Disable Screenshot Blocking

```cmd
ssapt.exe disable
```

### Check Status

```cmd
ssapt.exe status
```

## Testing

Try taking a screenshot while blocking is enabled:
- Press `PrtScn` or `Win+PrtScn`
- Use Snipping Tool
- Use third-party tools like ShareX

Screenshots should fail or appear black.

## Common Commands

| Command | Description |
|---------|-------------|
| `ssapt.exe enable` | Enable system-wide blocking |
| `ssapt.exe disable` | Disable blocking |
| `ssapt.exe status` | Check if blocking is active |
| `driver_manager.bat status` | Check driver service status |
| `driver_manager.bat stop` | Stop the driver |
| `driver_manager.bat start` | Start the driver |

## Troubleshooting

### "Failed to open SSAPT device"

The driver isn't running. Try:
```cmd
driver_manager.bat start
```

### "Access Denied"

You need Administrator privileges. Right-click Command Prompt and select "Run as Administrator".

### Driver Won't Load

1. Check test signing is enabled: `bcdedit | findstr testsigning`
2. Check Event Viewer for error details
3. Make sure you rebooted after enabling test signing

### Screenshots Still Work

1. Verify blocking is enabled: `ssapt.exe status`
2. Some screenshot methods may bypass kernel hooks
3. Check DebugView for hook activity

## Uninstallation

```cmd
ssapt.exe disable
driver_manager.bat stop
driver_manager.bat uninstall
```

Optionally disable test signing:
```cmd
bcdedit /set testsigning off
```

Then reboot.

## Debug Mode

To see kernel debug messages:

1. Download DebugView from Microsoft Sysinternals
2. Run as Administrator
3. Enable "Capture Kernel" in Capture menu
4. Look for `[SSAPT]` tagged messages

## Important Notes

⚠️ **Test Signing Mode**: Development requires test signing mode, which shows a watermark on your desktop.

⚠️ **System Stability**: Kernel drivers have full system access. If you experience issues, stop the driver immediately:
```cmd
driver_manager.bat stop
```

⚠️ **Production Use**: For production deployment, the driver must be properly signed with a valid certificate.

⚠️ **Kernel Development**: Modifying kernel code requires understanding of kernel programming and can cause system crashes if done incorrectly.

## Next Steps

- Read [BUILD.md](BUILD.md) for detailed build instructions
- Read [USAGE.md](USAGE.md) for advanced usage scenarios
- Read [ARCHITECTURE.md](ARCHITECTURE.md) to understand the design
- Read [TECHNICAL.md](TECHNICAL.md) for implementation details

## Getting Help

If you encounter issues:

1. Check the documentation files
2. Review Event Viewer (eventvwr.msc) → Windows Logs → System
3. Use DebugView to see kernel debug output
4. Ensure all prerequisites are met
5. Verify you're running with Administrator privileges

## Safety First

Always test in a virtual machine or non-production environment first. Kernel drivers can affect system stability if not properly developed and tested.
