// SSAPT Test Program
// Tests the screenshot blocking functionality

#include <windows.h>
#include <iostream>
#include "driver.h"

// Attempt to take a screenshot using GDI
bool TestGDIScreenshot() {
    std::cout << "\n[TEST] Attempting GDI screenshot..." << std::endl;
    
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
    
    BOOL result = BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);
    
    if (result) {
        std::cout << "[TEST] GDI screenshot succeeded (blocking may not be active)" << std::endl;
    } else {
        std::cout << "[TEST] GDI screenshot blocked successfully!" << std::endl;
    }
    
    SelectObject(hdcMem, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    
    return !result; // Return true if blocked
}

// Attempt to access frame buffer data
bool TestFrameBufferAccess() {
    std::cout << "\n[TEST] Attempting frame buffer access via GetDIBits..." << std::endl;
    
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    
    int width = 100;
    int height = 100;
    
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMem, hBitmap);
    
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    BYTE* buffer = new BYTE[width * height * 4];
    int result = GetDIBits(hdcMem, hBitmap, 0, height, buffer, &bmi, DIB_RGB_COLORS);
    
    if (result > 0) {
        std::cout << "[TEST] Frame buffer access succeeded (blocking may not be active)" << std::endl;
    } else {
        std::cout << "[TEST] Frame buffer access blocked successfully!" << std::endl;
    }
    
    delete[] buffer;
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    
    return (result == 0); // Return true if blocked
}

int main() {
    std::cout << "=== SSAPT Driver Test ===" << std::endl;
    std::cout << "Testing screenshot blocking functionality\n" << std::endl;
    
    // Test with blocking enabled
    std::cout << ">>> Testing with blocking ENABLED <<<" << std::endl;
    EnableBlocking();
    
    bool gdiBlocked = TestGDIScreenshot();
    bool fbBlocked = TestFrameBufferAccess();
    
    // Test with blocking disabled
    std::cout << "\n>>> Testing with blocking DISABLED <<<" << std::endl;
    DisableBlocking();
    
    bool gdiAllowed = !TestGDIScreenshot();
    bool fbAllowed = !TestFrameBufferAccess();
    
    // Summary
    std::cout << "\n=== Test Results ===" << std::endl;
    std::cout << "GDI BitBlt blocking: " << (gdiBlocked ? "PASS" : "FAIL") << std::endl;
    std::cout << "Frame buffer blocking: " << (fbBlocked ? "PASS" : "FAIL") << std::endl;
    std::cout << "GDI unblocking: " << (gdiAllowed ? "PASS" : "FAIL") << std::endl;
    std::cout << "Frame buffer unblocking: " << (fbAllowed ? "PASS" : "FAIL") << std::endl;
    
    bool allPassed = gdiBlocked && fbBlocked && gdiAllowed && fbAllowed;
    std::cout << "\nOverall: " << (allPassed ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << std::endl;
    
    return allPassed ? 0 : 1;
}
