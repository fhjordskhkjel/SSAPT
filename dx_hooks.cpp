// DirectX VTable Hooking Implementation
// Hooks DirectX Present and GetBuffer methods to block frame buffer access

#include <windows.h>
#include <d3d9.h>
#include <d3d11.h>
#include <dxgi.h>
#include <iostream>
#include <vector>

// VTable indices for DirectX methods
#define PRESENT_VTABLE_INDEX 17
#define GETFRONTBUFFERDATA_VTABLE_INDEX 32
#define DXGI_PRESENT_VTABLE_INDEX 8
#define DXGI_GETBUFFER_VTABLE_INDEX 9

// Original function pointers stored after hooking
static void* g_OriginalD3D9Present = nullptr;
static void* g_OriginalD3D9GetFrontBufferData = nullptr;
static void* g_OriginalDXGIPresent = nullptr;
static void* g_OriginalDXGIGetBuffer = nullptr;

// Global blocking flag (extern from driver.cpp)
extern bool g_BlockScreenshots;

// Type definitions for DirectX methods
typedef HRESULT(STDMETHODCALLTYPE* D3D9PresentFunc)(IDirect3DDevice9*, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*);
typedef HRESULT(STDMETHODCALLTYPE* D3D9GetFrontBufferDataFunc)(IDirect3DDevice9*, UINT, IDirect3DSurface9*);
typedef HRESULT(STDMETHODCALLTYPE* DXGIPresentFunc)(IDXGISwapChain*, UINT, UINT);
typedef HRESULT(STDMETHODCALLTYPE* DXGIGetBufferFunc)(IDXGISwapChain*, UINT, REFIID, void**);

// Helper function to change memory protection
bool SetMemoryProtection(void* address, size_t size, DWORD newProtection, DWORD* oldProtection) {
    return VirtualProtect(address, size, newProtection, oldProtection) != 0;
}

// VTable hooking function
bool HookVTableMethod(void** vtable, int index, void* hookFunc, void** originalFunc) {
    DWORD oldProtection;
    if (!SetMemoryProtection(&vtable[index], sizeof(void*), PAGE_READWRITE, &oldProtection)) {
        return false;
    }
    
    *originalFunc = vtable[index];
    vtable[index] = hookFunc;
    
    SetMemoryProtection(&vtable[index], sizeof(void*), oldProtection, &oldProtection);
    return true;
}

// Hooked DirectX 9 Present
HRESULT STDMETHODCALLTYPE HookedD3D9Present_VTable(IDirect3DDevice9* pDevice, CONST RECT* pSourceRect, 
                                                     CONST RECT* pDestRect, HWND hDestWindowOverride, 
                                                     CONST RGNDATA* pDirtyRegion) {
    if (g_BlockScreenshots) {
        std::cout << "[SSAPT] Monitored D3D9 Present call (vtable hook)" << std::endl;
    }
    
    D3D9PresentFunc originalFunc = (D3D9PresentFunc)g_OriginalD3D9Present;
    return originalFunc(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

// Hooked DirectX 9 GetFrontBufferData
HRESULT STDMETHODCALLTYPE HookedD3D9GetFrontBufferData_VTable(IDirect3DDevice9* pDevice, UINT iSwapChain, 
                                                                IDirect3DSurface9* pDestSurface) {
    if (g_BlockScreenshots) {
        std::cout << "[SSAPT] Blocked D3D9 GetFrontBufferData (vtable hook)" << std::endl;
        return D3DERR_INVALIDCALL;
    }
    
    D3D9GetFrontBufferDataFunc originalFunc = (D3D9GetFrontBufferDataFunc)g_OriginalD3D9GetFrontBufferData;
    return originalFunc(pDevice, iSwapChain, pDestSurface);
}

// Hooked DXGI Present
HRESULT STDMETHODCALLTYPE HookedDXGIPresent_VTable(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (g_BlockScreenshots) {
        std::cout << "[SSAPT] Monitored DXGI Present call (vtable hook)" << std::endl;
    }
    
    DXGIPresentFunc originalFunc = (DXGIPresentFunc)g_OriginalDXGIPresent;
    return originalFunc(pSwapChain, SyncInterval, Flags);
}

// Hooked DXGI GetBuffer
HRESULT STDMETHODCALLTYPE HookedDXGIGetBuffer_VTable(IDXGISwapChain* pSwapChain, UINT Buffer, 
                                                       REFIID riid, void** ppSurface) {
    if (g_BlockScreenshots) {
        std::cout << "[SSAPT] Blocked DXGI GetBuffer frame buffer access (vtable hook)" << std::endl;
        return E_FAIL;
    }
    
    DXGIGetBufferFunc originalFunc = (DXGIGetBufferFunc)g_OriginalDXGIGetBuffer;
    return originalFunc(pSwapChain, Buffer, riid, ppSurface);
}

// Initialize DirectX 9 hooks
bool InitializeD3D9Hooks() {
    // Create a temporary D3D9 device to get vtable
    HMODULE d3d9Module = LoadLibraryA("d3d9.dll");
    if (!d3d9Module) {
        std::cerr << "[SSAPT] Failed to load d3d9.dll" << std::endl;
        return false;
    }
    
    typedef IDirect3D9* (WINAPI* Direct3DCreate9Func)(UINT);
    Direct3DCreate9Func Direct3DCreate9Ptr = (Direct3DCreate9Func)GetProcAddress(d3d9Module, "Direct3DCreate9");
    
    if (!Direct3DCreate9Ptr) {
        FreeLibrary(d3d9Module);
        return false;
    }
    
    IDirect3D9* pD3D = Direct3DCreate9Ptr(D3D_SDK_VERSION);
    if (!pD3D) {
        FreeLibrary(d3d9Module);
        return false;
    }
    
    D3DPRESENT_PARAMETERS d3dpp = {0};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = GetDesktopWindow();
    
    IDirect3DDevice9* pDevice = nullptr;
    HRESULT hr = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, d3dpp.hDeviceWindow,
                                     D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice);
    
    if (SUCCEEDED(hr) && pDevice) {
        void** vtable = *(void***)pDevice;
        
        // Hook Present and GetFrontBufferData
        bool presentHooked = HookVTableMethod(vtable, PRESENT_VTABLE_INDEX, 
                                               HookedD3D9Present_VTable, &g_OriginalD3D9Present);
        bool getFBHooked = HookVTableMethod(vtable, GETFRONTBUFFERDATA_VTABLE_INDEX,
                                             HookedD3D9GetFrontBufferData_VTable, &g_OriginalD3D9GetFrontBufferData);
        
        pDevice->Release();
        pD3D->Release();
        
        if (presentHooked && getFBHooked) {
            std::cout << "[SSAPT] D3D9 hooks installed successfully" << std::endl;
            return true;
        }
    }
    
    if (pD3D) pD3D->Release();
    FreeLibrary(d3d9Module);
    return false;
}

// Initialize DirectX 11/DXGI hooks
bool InitializeDXGIHooks() {
    // Create a temporary DXGI swap chain to get vtable
    HMODULE dxgiModule = LoadLibraryA("dxgi.dll");
    HMODULE d3d11Module = LoadLibraryA("d3d11.dll");
    
    if (!dxgiModule || !d3d11Module) {
        std::cerr << "[SSAPT] Failed to load DXGI/D3D11 modules" << std::endl;
        return false;
    }
    
    typedef HRESULT (WINAPI* CreateDXGIFactoryFunc)(REFIID, void**);
    CreateDXGIFactoryFunc CreateDXGIFactoryPtr = (CreateDXGIFactoryFunc)GetProcAddress(dxgiModule, "CreateDXGIFactory");
    
    if (!CreateDXGIFactoryPtr) {
        FreeLibrary(dxgiModule);
        FreeLibrary(d3d11Module);
        return false;
    }
    
    IDXGIFactory* pFactory = nullptr;
    HRESULT hr = CreateDXGIFactoryPtr(__uuidof(IDXGIFactory), (void**)&pFactory);
    
    if (SUCCEEDED(hr) && pFactory) {
        IDXGIAdapter* pAdapter = nullptr;
        pFactory->EnumAdapters(0, &pAdapter);
        
        DXGI_SWAP_CHAIN_DESC scd = {0};
        scd.BufferCount = 1;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = GetDesktopWindow();
        scd.SampleDesc.Count = 1;
        scd.Windowed = TRUE;
        
        typedef HRESULT (WINAPI* D3D11CreateDeviceAndSwapChainFunc)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT,
                                                                      CONST D3D_FEATURE_LEVEL*, UINT, UINT,
                                                                      CONST DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**,
                                                                      ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
        
        D3D11CreateDeviceAndSwapChainFunc D3D11CreateDeviceAndSwapChainPtr = 
            (D3D11CreateDeviceAndSwapChainFunc)GetProcAddress(d3d11Module, "D3D11CreateDeviceAndSwapChain");
        
        if (D3D11CreateDeviceAndSwapChainPtr) {
            IDXGISwapChain* pSwapChain = nullptr;
            ID3D11Device* pDevice = nullptr;
            ID3D11DeviceContext* pContext = nullptr;
            
            hr = D3D11CreateDeviceAndSwapChainPtr(nullptr, D3D_DRIVER_TYPE_NULL, nullptr, 0, nullptr, 0,
                                                   D3D11_SDK_VERSION, &scd, &pSwapChain, &pDevice, nullptr, &pContext);
            
            if (SUCCEEDED(hr) && pSwapChain) {
                void** vtable = *(void***)pSwapChain;
                
                // Hook Present and GetBuffer
                bool presentHooked = HookVTableMethod(vtable, DXGI_PRESENT_VTABLE_INDEX,
                                                       HookedDXGIPresent_VTable, &g_OriginalDXGIPresent);
                bool getBufferHooked = HookVTableMethod(vtable, DXGI_GETBUFFER_VTABLE_INDEX,
                                                         HookedDXGIGetBuffer_VTable, &g_OriginalDXGIGetBuffer);
                
                if (pContext) pContext->Release();
                if (pDevice) pDevice->Release();
                if (pSwapChain) pSwapChain->Release();
                
                if (presentHooked && getBufferHooked) {
                    std::cout << "[SSAPT] DXGI hooks installed successfully" << std::endl;
                    pFactory->Release();
                    FreeLibrary(dxgiModule);
                    FreeLibrary(d3d11Module);
                    return true;
                }
            }
        }
        
        pFactory->Release();
    }
    
    FreeLibrary(dxgiModule);
    FreeLibrary(d3d11Module);
    return false;
}

// Initialize all DirectX hooks
bool InitializeDirectXHooks() {
    bool d3d9Success = InitializeD3D9Hooks();
    bool dxgiSuccess = InitializeDXGIHooks();
    
    return d3d9Success || dxgiSuccess; // Success if at least one initialized
}
