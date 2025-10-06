// SSAPT - Screenshot Anti-Protection Testing Driver
// This driver hooks into GDI and DirectX rendering paths to block screenshots

#include <windows.h>
#include <d3d9.h>
#include <d3d11.h>
#include <dxgi.h>
#include <detours.h>
#include <iostream>

// GDI Function Pointers
static BOOL (WINAPI* TrueBitBlt)(HDC, int, int, int, int, HDC, int, int, DWORD) = BitBlt;
static int (WINAPI* TrueGetDIBits)(HDC, HBITMAP, UINT, UINT, LPVOID, LPBITMAPINFO, UINT) = GetDIBits;
static HDC (WINAPI* TrueCreateCompatibleDC)(HDC) = CreateCompatibleDC;
static HBITMAP (WINAPI* TrueCreateCompatibleBitmap)(HDC, int, int) = CreateCompatibleBitmap;

// DirectX 9 Function Pointers
static HRESULT (STDMETHODCALLTYPE* TrueD3D9Present)(IDirect3DDevice9*, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*) = nullptr;
static HRESULT (STDMETHODCALLTYPE* TrueD3D9GetFrontBufferData)(IDirect3DDevice9*, UINT, IDirect3DSurface9*) = nullptr;

// DirectX 11 Function Pointers
static HRESULT (STDMETHODCALLTYPE* TrueD3D11Present)(IDXGISwapChain*, UINT, UINT) = nullptr;

// DXGI Function Pointers
static HRESULT (STDMETHODCALLTYPE* TrueDXGIGetBuffer)(IDXGISwapChain*, UINT, REFIID, void**) = nullptr;

// Global flag to enable/disable blocking
bool g_BlockScreenshots = true;

// Hooked GDI Functions
BOOL WINAPI HookedBitBlt(HDC hdcDest, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop) {
    if (g_BlockScreenshots) {
        std::cout << "[SSAPT] Blocked BitBlt screenshot attempt" << std::endl;
        return FALSE;
    }
    return TrueBitBlt(hdcDest, x, y, cx, cy, hdcSrc, x1, y1, rop);
}

int WINAPI HookedGetDIBits(HDC hdc, HBITMAP hbm, UINT start, UINT cLines, LPVOID lpvBits, LPBITMAPINFO lpbmi, UINT usage) {
    if (g_BlockScreenshots) {
        std::cout << "[SSAPT] Blocked GetDIBits screenshot attempt" << std::endl;
        return 0;
    }
    return TrueGetDIBits(hdc, hbm, start, cLines, lpvBits, lpbmi, usage);
}

HDC WINAPI HookedCreateCompatibleDC(HDC hdc) {
    if (g_BlockScreenshots) {
        std::cout << "[SSAPT] Monitored CreateCompatibleDC call" << std::endl;
    }
    return TrueCreateCompatibleDC(hdc);
}

HBITMAP WINAPI HookedCreateCompatibleBitmap(HDC hdc, int cx, int cy) {
    if (g_BlockScreenshots) {
        std::cout << "[SSAPT] Monitored CreateCompatibleBitmap call" << std::endl;
    }
    return TrueCreateCompatibleBitmap(hdc, cx, cy);
}

// Hooked DirectX 9 Functions
HRESULT STDMETHODCALLTYPE HookedD3D9Present(IDirect3DDevice9* pDevice, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
    if (g_BlockScreenshots) {
        std::cout << "[SSAPT] Monitored D3D9 Present call" << std::endl;
    }
    return TrueD3D9Present(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT STDMETHODCALLTYPE HookedD3D9GetFrontBufferData(IDirect3DDevice9* pDevice, UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
    if (g_BlockScreenshots) {
        std::cout << "[SSAPT] Blocked D3D9 GetFrontBufferData screenshot attempt" << std::endl;
        return D3DERR_INVALIDCALL;
    }
    return TrueD3D9GetFrontBufferData(pDevice, iSwapChain, pDestSurface);
}

// Hooked DirectX 11/DXGI Functions
HRESULT STDMETHODCALLTYPE HookedDXGIPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (g_BlockScreenshots) {
        std::cout << "[SSAPT] Monitored DXGI Present call" << std::endl;
    }
    return TrueD3D11Present(pSwapChain, SyncInterval, Flags);
}

HRESULT STDMETHODCALLTYPE HookedDXGIGetBuffer(IDXGISwapChain* pSwapChain, UINT Buffer, REFIID riid, void** ppSurface) {
    if (g_BlockScreenshots) {
        std::cout << "[SSAPT] Blocked DXGI GetBuffer frame buffer access" << std::endl;
        return E_FAIL;
    }
    return TrueDXGIGetBuffer(pSwapChain, Buffer, riid, ppSurface);
}

// Initialize hooks
bool InitializeHooks() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // Hook GDI functions
    DetourAttach(&(PVOID&)TrueBitBlt, HookedBitBlt);
    DetourAttach(&(PVOID&)TrueGetDIBits, HookedGetDIBits);
    DetourAttach(&(PVOID&)TrueCreateCompatibleDC, HookedCreateCompatibleDC);
    DetourAttach(&(PVOID&)TrueCreateCompatibleBitmap, HookedCreateCompatibleBitmap);

    LONG error = DetourTransactionCommit();
    
    if (error == NO_ERROR) {
        std::cout << "[SSAPT] GDI hooks installed successfully" << std::endl;
        return true;
    } else {
        std::cerr << "[SSAPT] Failed to install hooks: " << error << std::endl;
        return false;
    }
}

// Forward declaration from dx_hooks.cpp
extern bool InitializeDirectXHooks();

// Hook DirectX functions (requires runtime detection)
bool HookDirectX() {
    return InitializeDirectXHooks();
}

// Remove hooks
void RemoveHooks() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourDetach(&(PVOID&)TrueBitBlt, HookedBitBlt);
    DetourDetach(&(PVOID&)TrueGetDIBits, HookedGetDIBits);
    DetourDetach(&(PVOID&)TrueCreateCompatibleDC, HookedCreateCompatibleDC);
    DetourDetach(&(PVOID&)TrueCreateCompatibleBitmap, HookedCreateCompatibleBitmap);

    DetourTransactionCommit();
    std::cout << "[SSAPT] Hooks removed" << std::endl;
}

// Enable or disable screenshot blocking
void SetBlockingEnabled(bool enabled) {
    g_BlockScreenshots = enabled;
    std::cout << "[SSAPT] Screenshot blocking " << (enabled ? "enabled" : "disabled") << std::endl;
}

// DLL Entry Point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            InitializeHooks();
            HookDirectX();
            break;
        case DLL_PROCESS_DETACH:
            RemoveHooks();
            break;
    }
    return TRUE;
}

// Exported functions for controlling the driver
extern "C" {
    __declspec(dllexport) void EnableBlocking() {
        SetBlockingEnabled(true);
    }

    __declspec(dllexport) void DisableBlocking() {
        SetBlockingEnabled(false);
    }

    __declspec(dllexport) bool IsBlockingEnabled() {
        return g_BlockScreenshots;
    }
}
