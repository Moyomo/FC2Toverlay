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
    static RECT targetClient;

    static bool CreateDeviceD3D(HWND hWnd);
    static void CleanupDeviceD3D();
    static void CreateRenderTarget();
    static void CleanupRenderTarget();
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static BOOL IsWindowAlive();
    static BOOL IsWindowFocus(HWND hCurrentProcessWindow);
    static void MoveWindow(HWND hCurrentProcessWindow);

public:
    static HWND hTargetWindow;
    static DWORD dTargetPID;
    static DWORD dwUIAccessErr;

    static void RenderSettingsWindow();
    static void RenderOverlay();
    static bool SetTargetWindow();
};

#endif