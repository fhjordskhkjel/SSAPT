#!/usr/bin/env python3
"""
SSAPT DLL Injector
Simple script to inject ssapt.dll into a running process
"""

import sys
import ctypes
import os
from ctypes import wintypes

# Windows API constants
PROCESS_ALL_ACCESS = 0x1F0FFF
MEM_COMMIT = 0x1000
MEM_RESERVE = 0x2000
PAGE_READWRITE = 0x04

# Load Windows APIs
kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)

def inject_dll(process_id, dll_path):
    """
    Inject a DLL into a process
    
    Args:
        process_id: Target process ID
        dll_path: Full path to the DLL to inject
    
    Returns:
        True if successful, False otherwise
    """
    # Verify DLL exists
    if not os.path.exists(dll_path):
        print(f"Error: DLL not found at {dll_path}")
        return False
    
    # Get full path
    dll_path = os.path.abspath(dll_path)
    dll_bytes = dll_path.encode('utf-8') + b'\x00'
    
    print(f"Injecting {dll_path} into process {process_id}...")
    
    # Open target process
    h_process = kernel32.OpenProcess(PROCESS_ALL_ACCESS, False, process_id)
    if not h_process:
        print(f"Error: Failed to open process {process_id}")
        print(f"Error code: {ctypes.get_last_error()}")
        return False
    
    try:
        # Allocate memory in target process
        mem_address = kernel32.VirtualAllocEx(
            h_process,
            None,
            len(dll_bytes),
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE
        )
        
        if not mem_address:
            print("Error: Failed to allocate memory in target process")
            return False
        
        # Write DLL path to allocated memory
        bytes_written = wintypes.SIZE_T(0)
        success = kernel32.WriteProcessMemory(
            h_process,
            mem_address,
            dll_bytes,
            len(dll_bytes),
            ctypes.byref(bytes_written)
        )
        
        if not success:
            print("Error: Failed to write DLL path to target process")
            return False
        
        # Get address of LoadLibraryA
        h_kernel32 = kernel32.GetModuleHandleA(b"kernel32.dll")
        load_library_addr = kernel32.GetProcAddress(h_kernel32, b"LoadLibraryA")
        
        if not load_library_addr:
            print("Error: Failed to get LoadLibraryA address")
            return False
        
        # Create remote thread to load the DLL
        thread_id = wintypes.DWORD(0)
        h_thread = kernel32.CreateRemoteThread(
            h_process,
            None,
            0,
            load_library_addr,
            mem_address,
            0,
            ctypes.byref(thread_id)
        )
        
        if not h_thread:
            print("Error: Failed to create remote thread")
            return False
        
        # Wait for thread to complete
        kernel32.WaitForSingleObject(h_thread, 0xFFFFFFFF)
        
        # Get thread exit code
        exit_code = wintypes.DWORD(0)
        kernel32.GetExitCodeThread(h_thread, ctypes.byref(exit_code))
        
        # Clean up
        kernel32.CloseHandle(h_thread)
        kernel32.VirtualFreeEx(h_process, mem_address, 0, 0x8000)  # MEM_RELEASE
        
        if exit_code.value:
            print(f"Success! DLL injected with module handle: 0x{exit_code.value:X}")
            return True
        else:
            print("Error: LoadLibrary returned NULL")
            return False
            
    finally:
        kernel32.CloseHandle(h_process)

def main():
    """Main entry point"""
    if len(sys.argv) < 2:
        print("SSAPT DLL Injector")
        print("Usage: python inject.py <process_id> [dll_path]")
        print()
        print("Example: python inject.py 1234 ssapt.dll")
        print("         python inject.py 1234 C:\\path\\to\\ssapt.dll")
        return 1
    
    try:
        process_id = int(sys.argv[1])
    except ValueError:
        print(f"Error: Invalid process ID '{sys.argv[1]}'")
        return 1
    
    # Default to ssapt.dll in current directory
    dll_path = sys.argv[2] if len(sys.argv) > 2 else "ssapt.dll"
    
    # Check if running as administrator
    try:
        is_admin = ctypes.windll.shell32.IsUserAnAdmin()
    except:
        is_admin = False
    
    if not is_admin:
        print("Warning: Not running as administrator. Injection may fail.")
        print("Try running: python inject.py <pid> <dll> (as administrator)")
    
    success = inject_dll(process_id, dll_path)
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
