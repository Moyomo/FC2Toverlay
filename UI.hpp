#ifndef UI_HPP
#define UI_HPP

#include "pch.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class UI
{
private:
    static ID3D11Device* pd3dDevice;
    static ID3D11DeviceContext* pd3dDeviceContext;
    static IDXGISwapChain* pSwapChain;
    static ID3D11RenderTargetView* pMainRenderTargetView;
    static HWND hTargetWindow;
    static DWORD dTargetPID;
    static RECT targetClient;
    static BOOL bStreamProof;
    static std::chrono::microseconds targetFrametime;
    static BOOL bDebug;

    static bool CreateDeviceD3D(HWND hWnd);
    static void CleanupDeviceD3D();
    static void CreateRenderTarget();
    static void CleanupRenderTarget();
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static BOOL IsWindowAlive();
    static BOOL IsWindowFocus(HWND hCurrentProcessWindow);
    static void MoveWindow(HWND hCurrentProcessWindow);

public:
    static void Render();
    static bool SetTargetWindow();
    static void GetConfigSettings();
};

#endif