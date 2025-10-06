// SSAPT Control Application
// User-mode application to control the kernel driver

#include <windows.h>
#include <iostream>
#include <string>

// IOCTL codes (must match kernel driver)
#define IOCTL_SSAPT_ENABLE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SSAPT_DISABLE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SSAPT_STATUS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Device symbolic link
#define DEVICE_LINK L"\\\\.\\SSAPT"

class SSAPTController {
private:
    HANDLE hDevice;
    
    bool OpenDevice() {
        hDevice = CreateFileW(
            DEVICE_LINK,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        
        if (hDevice == INVALID_HANDLE_VALUE) {
            std::cerr << "Error: Failed to open SSAPT device" << std::endl;
            std::cerr << "Make sure the kernel driver is loaded" << std::endl;
            std::cerr << "Error code: " << GetLastError() << std::endl;
            return false;
        }
        
        return true;
    }
    
    void CloseDevice() {
        if (hDevice != INVALID_HANDLE_VALUE) {
            CloseHandle(hDevice);
            hDevice = INVALID_HANDLE_VALUE;
        }
    }
    
    bool SendIoctl(DWORD ioctlCode, PVOID inputBuffer = NULL, DWORD inputSize = 0, 
                   PVOID outputBuffer = NULL, DWORD outputSize = 0, PDWORD bytesReturned = NULL) {
        DWORD bytes = 0;
        BOOL result = DeviceIoControl(
            hDevice,
            ioctlCode,
            inputBuffer,
            inputSize,
            outputBuffer,
            outputSize,
            &bytes,
            NULL
        );
        
        if (bytesReturned) {
            *bytesReturned = bytes;
        }
        
        if (!result) {
            std::cerr << "IOCTL failed with error: " << GetLastError() << std::endl;
            return false;
        }
        
        return true;
    }
    
public:
    SSAPTController() : hDevice(INVALID_HANDLE_VALUE) {}
    
    ~SSAPTController() {
        CloseDevice();
    }
    
    bool Enable() {
        if (!OpenDevice()) {
            return false;
        }
        
        bool result = SendIoctl(IOCTL_SSAPT_ENABLE);
        CloseDevice();
        
        if (result) {
            std::cout << "[SSAPT] Screenshot blocking ENABLED" << std::endl;
        }
        
        return result;
    }
    
    bool Disable() {
        if (!OpenDevice()) {
            return false;
        }
        
        bool result = SendIoctl(IOCTL_SSAPT_DISABLE);
        CloseDevice();
        
        if (result) {
            std::cout << "[SSAPT] Screenshot blocking DISABLED" << std::endl;
        }
        
        return result;
    }
    
    bool GetStatus(bool* pEnabled) {
        if (!OpenDevice()) {
            return false;
        }
        
        BOOLEAN status = FALSE;
        DWORD bytesReturned = 0;
        bool result = SendIoctl(IOCTL_SSAPT_STATUS, NULL, 0, &status, sizeof(status), &bytesReturned);
        CloseDevice();
        
        if (result && bytesReturned == sizeof(BOOLEAN)) {
            *pEnabled = (status != FALSE);
            return true;
        }
        
        return false;
    }
    
    void ShowStatus() {
        bool enabled = false;
        if (GetStatus(&enabled)) {
            if (enabled) {
                std::cout << "[SSAPT] Status: ENABLED - Screenshots are blocked" << std::endl;
            } else {
                std::cout << "[SSAPT] Status: DISABLED - Screenshots are allowed" << std::endl;
            }
        } else {
            std::cerr << "Failed to get status" << std::endl;
        }
    }
};

void ShowUsage(const char* programName) {
    std::cout << "SSAPT Control Application" << std::endl;
    std::cout << "=========================" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: " << programName << " <command>" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  enable   - Enable system-wide screenshot blocking" << std::endl;
    std::cout << "  disable  - Disable system-wide screenshot blocking" << std::endl;
    std::cout << "  status   - Show current blocking status" << std::endl;
    std::cout << "  help     - Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " enable" << std::endl;
    std::cout << "  " << programName << " disable" << std::endl;
    std::cout << "  " << programName << " status" << std::endl;
    std::cout << std::endl;
    std::cout << "Note: This application requires the SSAPT kernel driver to be loaded." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        ShowUsage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    SSAPTController controller;
    
    if (command == "enable") {
        return controller.Enable() ? 0 : 1;
    } else if (command == "disable") {
        return controller.Disable() ? 0 : 1;
    } else if (command == "status") {
        controller.ShowStatus();
        return 0;
    } else if (command == "help") {
        ShowUsage(argv[0]);
        return 0;
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
        std::cout << "Use '" << argv[0] << " help' for usage information" << std::endl;
        return 1;
    }
    
    return 0;
}
