# Building SSAPT

## Prerequisites

- Windows 10 or later (Windows 11 recommended)
- Visual Studio 2019 or later with C++ development tools
- **Windows Driver Kit (WDK)** - Required for kernel driver
- CMake 3.10 or later (for control application)
- Administrator privileges

## Build Instructions

### Part 1: Build the Control Application (User-Mode)

The control application is built using CMake:

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Generate build files:
```bash
cmake ..
```

3. Build the project:
```bash
cmake --build . --config Release
```

The output will be:
- `ssapt.exe` - The control application

### Part 2: Build the Kernel Driver

The kernel driver requires the Windows Driver Kit (WDK).

#### Option A: Using Visual Studio with WDK Integration

1. Install WDK (download from Microsoft)
2. Open Visual Studio
3. File → New → Project → Kernel Mode Driver, Empty (KMDF)
4. Add `kernel_driver.c` to the project
5. Set project properties:
   - Configuration Type: Driver
   - Driver Type: WDM
6. Build the solution (Ctrl+Shift+B)

The output will be:
- `ssapt.sys` - The kernel driver
- `ssapt.inf` - Driver installation file

#### Option B: Using WDK Build Environment

1. Open a WDK command prompt (as Administrator)
2. Navigate to the SSAPT directory
3. Run the build command:
```cmd
build -ceZ
```

Or use the legacy build system:
```cmd
cd /d C:\path\to\SSAPT
set DRIVER_NAME=ssapt
msbuild ssapt.vcxproj /p:Configuration=Release /p:Platform=x64
```

## Installation

### Step 1: Enable Test Signing (Development Only)

For development/testing without a signed driver:

```cmd
bcdedit /set testsigning on
```

**Reboot your computer** after enabling test signing.

### Step 2: Install the Kernel Driver

Run as Administrator:

```cmd
driver_manager.bat install
driver_manager.bat start
```

Verify the driver is running:
```cmd
driver_manager.bat status
```

### Step 3: Test the Control Application

```cmd
ssapt.exe status
ssapt.exe enable
ssapt.exe disable
```

## Usage

### Enable Screenshot Blocking

```cmd
ssapt.exe enable
```

### Disable Screenshot Blocking

```cmd
ssapt.exe disable
```

### Check Current Status

```cmd
ssapt.exe status
```

### Stop and Uninstall

```cmd
driver_manager.bat stop
driver_manager.bat uninstall
```

## Troubleshooting

### Build Errors

**Control Application:**
- **CMake not found**: Install CMake and add to PATH
- **Compiler not found**: Install Visual Studio with C++ tools
- **Link errors**: Ensure Windows SDK is installed

**Kernel Driver:**
- **WDK not found**: Install Windows Driver Kit from Microsoft
- **Build tools missing**: Install WDK along with Visual Studio
- **Target platform errors**: Check WDK version matches your Windows version

### Installation Issues

- **Driver installation fails**: Run `driver_manager.bat` as Administrator
- **Test signing error**: Enable test signing with `bcdedit /set testsigning on` and reboot
- **Access denied**: Ensure you have Administrator privileges
- **Driver won't load**: Check Windows Event Viewer for detailed error messages

### Runtime Issues

- **Control app can't connect**: Ensure the driver is installed and running
  ```cmd
  driver_manager.bat status
  ```
- **Commands not working**: Check driver logs in DebugView or Event Viewer
- **System instability**: Stop the driver immediately:
  ```cmd
  driver_manager.bat stop
  ```

## Debug Tools

### DebugView

Download DebugView from Microsoft Sysinternals to see kernel debug output:

1. Run DebugView as Administrator
2. Enable "Capture Kernel" in the Capture menu
3. Watch for `[SSAPT]` tagged messages

### Event Viewer

Check Windows Event Viewer for driver events:
1. Open Event Viewer (eventvwr.msc)
2. Navigate to Windows Logs → System
3. Filter by source "SSAPT" or look for Service Control Manager events

## Notes

- **Development Only**: This driver is for testing and research purposes
- **Test Signing**: Required for unsigned drivers during development
- **Administrator Rights**: Required for all driver operations
- **Kernel Safety**: All operations include proper error handling
- **System Stability**: The driver includes safety checks to prevent crashes
- **Compatibility**: Designed for Windows 10/11 x64 systems

## Production Deployment

For production use:

1. **Sign the driver** with a valid code signing certificate
2. Submit to Microsoft for attestation signing
3. Disable test signing mode
4. Use proper installation package (MSI/INF)
5. Include proper version information and digital signatures
