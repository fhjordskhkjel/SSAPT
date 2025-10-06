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

// Helper function to validate memory pointer
bool IsValidMemoryPtr(void* ptr, size_t size) {
    if (!ptr) return false;
    
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0) {
        return false;
    }
    
    // Check if memory is committed and accessible
    if (mbi.State != MEM_COMMIT) {
        return false;
    }
    
    return true;
}

// Helper function to change memory protection
bool SetMemoryProtection(void* address, size_t size, DWORD newProtection, DWORD* oldProtection) {
    if (!address || !IsValidMemoryPtr(address, size)) {
        return false;
    }
    return VirtualProtect(address, size, newProtection, oldProtection) != 0;
}

// VTable hooking function
bool HookVTableMethod(void** vtable, int index, void* hookFunc, void** originalFunc) {
    __try {
        // Validate all pointers before use
        if (!vtable || !hookFunc || !originalFunc) {
            std::cerr << "[SSAPT] Invalid pointer in HookVTableMethod" << std::endl;
            return false;
        }
        
        // Validate index is reasonable
        if (index < 0 || index > 200) {
            std::cerr << "[SSAPT] Invalid vtable index: " << index << std::endl;
            return false;
        }
        
        // Validate vtable entry is readable
        if (!IsValidMemoryPtr(&vtable[index], sizeof(void*))) {
            std::cerr << "[SSAPT] Invalid vtable entry at index " << index << std::endl;
            return false;
        }
        
        DWORD oldProtection;
        if (!SetMemoryProtection(&vtable[index], sizeof(void*), PAGE_READWRITE, &oldProtection)) {
            std::cerr << "[SSAPT] Failed to change memory protection" << std::endl;
            return false;
        }
        
        *originalFunc = vtable[index];
        vtable[index] = hookFunc;
        
        SetMemoryProtection(&vtable[index], sizeof(void*), oldProtection, &oldProtection);
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        std::cerr << "[SSAPT] Exception in HookVTableMethod" << std::endl;
        return false;
    }
}

// Hooked DirectX 9 Present
HRESULT STDMETHODCALLTYPE HookedD3D9Present_VTable(IDirect3DDevice9* pDevice, CONST RECT* pSourceRect, 
                                                     CONST RECT* pDestRect, HWND hDestWindowOverride, 
                                                     CONST RGNDATA* pDirtyRegion) {
    __try {
        // Validate device and function pointer
        if (!pDevice || !g_OriginalD3D9Present) {
            return D3DERR_INVALIDCALL;
        }
        
        if (g_BlockScreenshots) {
            std::cout << "[SSAPT] Monitored D3D9 Present call (vtable hook)" << std::endl;
        }
        
        D3D9PresentFunc originalFunc = (D3D9PresentFunc)g_OriginalD3D9Present;
        return originalFunc(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        std::cerr << "[SSAPT] Exception in HookedD3D9Present_VTable" << std::endl;
        return D3DERR_INVALIDCALL;
    }
}

// Hooked DirectX 9 GetFrontBufferData
HRESULT STDMETHODCALLTYPE HookedD3D9GetFrontBufferData_VTable(IDirect3DDevice9* pDevice, UINT iSwapChain, 
                                                                IDirect3DSurface9* pDestSurface) {
    __try {
        // Validate device and function pointer
        if (!pDevice || !g_OriginalD3D9GetFrontBufferData) {
            return D3DERR_INVALIDCALL;
        }
        
        if (g_BlockScreenshots) {
            std::cout << "[SSAPT] Blocked D3D9 GetFrontBufferData (vtable hook)" << std::endl;
            return D3DERR_INVALIDCALL;
        }
        
        D3D9GetFrontBufferDataFunc originalFunc = (D3D9GetFrontBufferDataFunc)g_OriginalD3D9GetFrontBufferData;
        return originalFunc(pDevice, iSwapChain, pDestSurface);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        std::cerr << "[SSAPT] Exception in HookedD3D9GetFrontBufferData_VTable" << std::endl;
        return D3DERR_INVALIDCALL;
    }
}

// Hooked DXGI Present
HRESULT STDMETHODCALLTYPE HookedDXGIPresent_VTable(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    __try {
        // Validate swap chain and function pointer
        if (!pSwapChain || !g_OriginalDXGIPresent) {
            return E_FAIL;
        }
        
        if (g_BlockScreenshots) {
            std::cout << "[SSAPT] Monitored DXGI Present call (vtable hook)" << std::endl;
        }
        
        DXGIPresentFunc originalFunc = (DXGIPresentFunc)g_OriginalDXGIPresent;
        return originalFunc(pSwapChain, SyncInterval, Flags);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        std::cerr << "[SSAPT] Exception in HookedDXGIPresent_VTable" << std::endl;
        return E_FAIL;
    }
}

// Hooked DXGI GetBuffer
HRESULT STDMETHODCALLTYPE HookedDXGIGetBuffer_VTable(IDXGISwapChain* pSwapChain, UINT Buffer, 
                                                       REFIID riid, void** ppSurface) {
    __try {
        // Validate swap chain and function pointer
        if (!pSwapChain || !g_OriginalDXGIGetBuffer) {
            return E_FAIL;
        }
        
        if (g_BlockScreenshots) {
            std::cout << "[SSAPT] Blocked DXGI GetBuffer frame buffer access (vtable hook)" << std::endl;
            return E_FAIL;
        }
        
        DXGIGetBufferFunc originalFunc = (DXGIGetBufferFunc)g_OriginalDXGIGetBuffer;
        return originalFunc(pSwapChain, Buffer, riid, ppSurface);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        std::cerr << "[SSAPT] Exception in HookedDXGIGetBuffer_VTable" << std::endl;
        return E_FAIL;
    }
}

// Initialize DirectX 9 hooks
bool InitializeD3D9Hooks() {
    __try {
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
            // Validate device vtable pointer
            if (!IsValidMemoryPtr(pDevice, sizeof(void*))) {
                std::cerr << "[SSAPT] Invalid device pointer" << std::endl;
                pDevice->Release();
                pD3D->Release();
                FreeLibrary(d3d9Module);
                return false;
            }
            
            void** vtable = *(void***)pDevice;
            
            // Validate vtable pointer
            if (!IsValidMemoryPtr(vtable, sizeof(void*) * 50)) {
                std::cerr << "[SSAPT] Invalid vtable pointer" << std::endl;
                pDevice->Release();
                pD3D->Release();
                FreeLibrary(d3d9Module);
                return false;
            }
            
            // Hook Present and GetFrontBufferData
            bool presentHooked = HookVTableMethod(vtable, PRESENT_VTABLE_INDEX, 
                                                   HookedD3D9Present_VTable, &g_OriginalD3D9Present);
            bool getFBHooked = HookVTableMethod(vtable, GETFRONTBUFFERDATA_VTABLE_INDEX,
                                                 HookedD3D9GetFrontBufferData_VTable, &g_OriginalD3D9GetFrontBufferData);
            
            pDevice->Release();
            pD3D->Release();
            
            if (presentHooked && getFBHooked) {
                std::cout << "[SSAPT] D3D9 hooks installed successfully" << std::endl;
                FreeLibrary(d3d9Module);
                return true;
            }
        }
        
        if (pD3D) pD3D->Release();
        FreeLibrary(d3d9Module);
        return false;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        std::cerr << "[SSAPT] Critical exception in InitializeD3D9Hooks" << std::endl;
        return false;
    }
}

// Initialize DirectX 11/DXGI hooks
bool InitializeDXGIHooks() {
    __try {
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
                    // Validate swap chain vtable pointer
                    if (!IsValidMemoryPtr(pSwapChain, sizeof(void*))) {
                        std::cerr << "[SSAPT] Invalid swap chain pointer" << std::endl;
                        if (pContext) pContext->Release();
                        if (pDevice) pDevice->Release();
                        if (pSwapChain) pSwapChain->Release();
                        pFactory->Release();
                        FreeLibrary(dxgiModule);
                        FreeLibrary(d3d11Module);
                        return false;
                    }
                    
                    void** vtable = *(void***)pSwapChain;
                    
                    // Validate vtable pointer
                    if (!IsValidMemoryPtr(vtable, sizeof(void*) * 20)) {
                        std::cerr << "[SSAPT] Invalid vtable pointer" << std::endl;
                        if (pContext) pContext->Release();
                        if (pDevice) pDevice->Release();
                        if (pSwapChain) pSwapChain->Release();
                        pFactory->Release();
                        FreeLibrary(dxgiModule);
                        FreeLibrary(d3d11Module);
                        return false;
                    }
                    
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
    __except(EXCEPTION_EXECUTE_HANDLER) {
        std::cerr << "[SSAPT] Critical exception in InitializeDXGIHooks" << std::endl;
        return false;
    }
}

// Initialize all DirectX hooks
bool InitializeDirectXHooks() {
    __try {
        bool d3d9Success = InitializeD3D9Hooks();
        bool dxgiSuccess = InitializeDXGIHooks();
        
        return d3d9Success || dxgiSuccess; // Success if at least one initialized
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        std::cerr << "[SSAPT] Critical exception in InitializeDirectXHooks" << std::endl;
        return false;
    }
}
