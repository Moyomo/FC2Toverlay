#include "UI.hpp"
#include "Drawing.hpp"
#include "uiaccess.hpp"
#include "Config.hpp"

// define default values
ID3D11Device* UI::pd3dDevice = nullptr;
ID3D11DeviceContext* UI::pd3dDeviceContext = nullptr;
IDXGISwapChain* UI::pSwapChain = nullptr;
ID3D11RenderTargetView* UI::pMainRenderTargetView = nullptr;
HWND UI::hTargetWindow = nullptr;
DWORD UI::dTargetPID = 0;
DWORD UI::dwUIAccessErr = ERROR_NOT_FOUND;
RECT UI::targetClient = {};
DWORD UI::dwWindowStyles = WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;

// const variables
const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
const auto microsecond = std::chrono::microseconds(1);
const auto millisecond = std::chrono::microseconds(1000);

// relevant ZBIDs
enum ZBID
{
    ZBID_DEFAULT = 0,
    ZBID_DESKTOP = 1,
    ZBID_UIACCESS = 2
};

// undocumented WinAPI function to create windows in specified band
typedef HWND(WINAPI* CreateWindowInBand)(_In_ DWORD dwExStyle, _In_opt_ LPCWSTR atom, _In_opt_ LPCWSTR lpWindowName, _In_ DWORD dwStyle, _In_ int X, _In_ int Y, _In_ int nWidth, _In_ int nHeight, _In_opt_ HWND hWndParent, _In_opt_ HMENU hMenu, _In_opt_ HINSTANCE hInstance, _In_opt_ LPVOID lpParam, DWORD band);

/**
 * @brief Create a D3D11 device
 * @param hWnd handle to the overlay window
 * @return true if the device has been created, otherwise false
 */
bool UI::CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, &featureLevel, &pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, &featureLevel, &pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

/**
 * @brief Create the render target
 */
void UI::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer != nullptr)
    {
        pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pMainRenderTargetView);
        pBackBuffer->Release();
    }
}

/**
 * @brief Release the render target
 */
void UI::CleanupRenderTarget()
{
    if (pMainRenderTargetView) { pMainRenderTargetView->Release(); pMainRenderTargetView = nullptr; }
}

/**
 * @brief Release the D3D11 device
 */
void UI::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (pSwapChain) { pSwapChain->Release(); pSwapChain = nullptr; }
    if (pd3dDeviceContext) { pd3dDeviceContext->Release(); pd3dDeviceContext = nullptr; }
    if (pd3dDevice) { pd3dDevice->Release(); pd3dDevice = nullptr; }
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

/**
 * @brief Callback function that processes messages sent to a window
 * https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc
 */
LRESULT WINAPI UI::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;

    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;

    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;

    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;

    default:
        break;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

/**
 * @brief Create settings window after application was launched
 */
void UI::RenderSettingsWindow()
{
#ifndef _DEBUG
    // get UIAccess so we can draw on top of fullscreen windows
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    UI::dwUIAccessErr = PrepareForUIAccess();
#endif // !_DEBUG

    // tell windows that our application is DPI aware to prevent automatic scaling
    ImGui_ImplWin32_EnableDpiAwareness();

    // check if there's a connection to Constellation
    Config::IsConstellationConnected();

    // skip settings window if the autostart option is enabled
    if (Config::bAutostart && UI::SetTargetWindow())
    {
        Config::bCreateOverlay = true;
        Config::SetRandomDimensions();
        return;
    }

    // create window class and window for the overlay settings
    const WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, _T("OverlaySettings"), nullptr };
    ::RegisterClassEx(&wc);
    const HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("OverlaySettings"), WS_OVERLAPPEDWINDOW, 100, 100, 50, 50, NULL, NULL, wc.hInstance, NULL);

    // create D3D11 device
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        MessageBox(
            NULL,
            (LPCWSTR)L"Can't create D3D11 device. Make sure your GPU supports Direct3D 10 or 11 feature levels.",
            (LPCWSTR)L"DirectX 11 Error",
            MB_ICONWARNING | MB_OK
        );
        return;
    }

    // hide the main window so only our ImGui window shows up
    ::ShowWindow(hwnd, SW_HIDE);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    io.Fonts->AddFontDefault();
    io.IniFilename = nullptr;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(pd3dDevice, pd3dDeviceContext);

    bool bDone = false;

    while (!bDone)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                bDone = true;
        }

        // check if the user pressed the exit key
        if (LI_FN(GetAsyncKeyState).in_cached(LI_MODULE("User32.dll").cached())(VK_END) & 1)
            bDone = true;

        if (bDone)
            break;

        // create a new frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            Drawing::DrawSettings();
        }
        ImGui::EndFrame();

        ImGui::Render();
        pd3dDeviceContext->OMSetRenderTargets(1, &pMainRenderTargetView, nullptr);
        pd3dDeviceContext->ClearRenderTargetView(pMainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        // present current frame on screen
        pSwapChain->Present(1, 0);

        // check if the settings window was closed
        if (!Drawing::IsActive())
            break;
    }

    // cleanup and shutdown
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    // clear window messages
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {}
}

/**
 * @brief Create overlay window and enter main loop
 */
void UI::RenderOverlay()
{
    // create window class for the overlay
    const WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_VREDRAW | CS_HREDRAW, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, _T("FC2Toverlay"), nullptr };
    ::RegisterClassEx(&wc);

    // create window in highest z-order possible
    DWORD band = UI::dwUIAccessErr == ERROR_SUCCESS ? ZBID_UIACCESS : ZBID_DESKTOP;

#ifdef _DEBUG
    // only add the topmost window style if there is no UIAccess
    if (UI::dwUIAccessErr != ERROR_SUCCESS)
        UI::dwWindowStyles |= WS_EX_TOPMOST;
#endif // _DEBUG

    // create overlay window
    const HWND hwnd = LI_FN_DEF(CreateWindowInBand).in(LI_MODULE("User32.dll").cached())(UI::dwWindowStyles, wc.lpszClassName, _T("FC2Toverlay"), WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, wc.hInstance, nullptr, band);

    // set display affinity to hide the window in screen captures
    if (Config::bStreamProof)
        SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);

    // add layered window to the stored window styles
    UI::dwWindowStyles |= WS_EX_LAYERED;

    // set window styles again because it can't be a layered window when setting the display affinity
    SetWindowLong(hwnd, GWL_EXSTYLE, UI::dwWindowStyles);

    // set window transparency
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    const MARGINS margin = { -1, 0, 0, 0 };
    DwmExtendFrameIntoClientArea(hwnd, &margin);

    // create D3D11 device
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        MessageBox(
            NULL,
            (LPCWSTR)L"Can't create D3D11 device. Make sure your GPU supports Direct3D 10 or 11 feature levels.",
            (LPCWSTR)L"DirectX 11 Error",
            MB_ICONWARNING | MB_OK
        );
        return;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(pd3dDevice, pd3dDeviceContext);

    bool bDone = false;

    // overlay loop
    while (!bDone)
    {
        // start timer for frametime calculations
        auto frame_start = std::chrono::high_resolution_clock::now();

        // loop over window messages and check for quit message
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                bDone = true;
        }

        // check if the user pressed the exit key
        if (LI_FN(GetAsyncKeyState).in_cached(LI_MODULE("User32.dll").cached())(VK_END) & 1)
            bDone = true;

        // check for the last FC2 error message
        if (fc2::get_error() != FC2_TEAM_ERROR_NO_ERROR)
            bDone = true;

        // check if the target window got closed
        if (!IsWindowAlive())
            bDone = true;

        if (bDone)
            break;

        // clear overlay when the target window is not in focus
        if (!IsWindowFocus(hwnd))
        {
            pd3dDeviceContext->OMSetRenderTargets(1, &pMainRenderTargetView, nullptr);
            pd3dDeviceContext->ClearRenderTargetView(pMainRenderTargetView, clear_color);

            pSwapChain->Present(4, 0);
            Sleep(250);

            continue;
        }

        // create new frame and get drawing requests
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            Drawing::Draw();
        }
        ImGui::EndFrame();

        ImGui::Render();
        pd3dDeviceContext->OMSetRenderTargets(1, &pMainRenderTargetView, nullptr);
        pd3dDeviceContext->ClearRenderTargetView(pMainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // present current frame on screen
        pSwapChain->Present(0, 0);

        // move the overlay on top of the target window
        MoveWindow(hwnd);

        // calculate the time we have to wait for to achieve our target frametime
        auto frame_end = std::chrono::high_resolution_clock::now();
        auto frame_time = std::chrono::duration_cast<std::chrono::microseconds>(frame_end - frame_start);
        auto time_to_wait = Config::targetFrametime - frame_time - millisecond;
        if (time_to_wait < microsecond)
            time_to_wait = microsecond;

        // NOTE: sleeping adds at least 1ms of waiting time by default
        std::this_thread::sleep_for(time_to_wait);
    }

    // cleanup and shutdown
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
}

/**
 * @brief Get the target window handle and save it
 * @return true if the target window is valid and the handle was saved, otherwise false
 */
bool UI::SetTargetWindow()
{
    if (fc2::get_error() != FC2_TEAM_ERROR_NO_ERROR)
        return false;

    uint32_t iTargetHandle = fc2::call<uint32_t>("directx_overlay_target_handle", FC2_LUA_TYPE_INT);
    if (iTargetHandle == 0)
        return false;
    uint32_t iTargetProcID = fc2::call<uint32_t>("directx_overlay_target_process_id", FC2_LUA_TYPE_INT);
    if (iTargetProcID == 0)
        return false;

    HWND hWindow = (HWND)IntToPtr(iTargetHandle);
    if (!LI_FN(IsWindow).in_cached(LI_MODULE("User32.dll").cached())(hWindow))
        return false;
    hTargetWindow = hWindow;
    LI_FN(GetWindowThreadProcessId).in_cached(LI_MODULE("User32.dll").cached())(hTargetWindow, &dTargetPID);
    return true;
}

/**
 * @brief Check if the target window got closed
 * @return TRUE if the window is still open, FALSE if it got closed
 */
BOOL UI::IsWindowAlive()
{
    DWORD dCurrentPID;

    if (hTargetWindow == nullptr)
        return FALSE;

    if (!LI_FN(IsWindow).in_cached(LI_MODULE("User32.dll").cached())(hTargetWindow))
        return FALSE;

    LI_FN(GetWindowThreadProcessId).in_cached(LI_MODULE("User32.dll").cached())(hTargetWindow, &dCurrentPID);

    if (dCurrentPID != dTargetPID)
        return FALSE;

    return TRUE;
}

/**
 * @brief Check if the overlay window or the target window is in focus
 * @param hCurrentProcessWindow Handle of the overlay window
 * @return TRUE if one of the windows is in focus, otherwise FALSE
 */
BOOL UI::IsWindowFocus(const HWND hCurrentProcessWindow)
{
    char lpCurrentWindowUsedClass[125];
    char lpCurrentWindowClass[125];
    char lpOverlayWindowClass[125];

    const HWND hCurrentWindowUsed = LI_FN(GetForegroundWindow).in_cached(LI_MODULE("User32.dll").cached())();
    if (LI_FN(GetClassNameA).in_cached(LI_MODULE("User32.dll").cached())(hCurrentWindowUsed, lpCurrentWindowUsedClass, sizeof(lpCurrentWindowUsedClass)) == 0)
        return FALSE;

    if (LI_FN(GetClassNameA).in_cached(LI_MODULE("User32.dll").cached())(hTargetWindow, lpCurrentWindowClass, sizeof(lpCurrentWindowClass)) == 0)
        return FALSE;

    if (LI_FN(GetClassNameA).in_cached(LI_MODULE("User32.dll").cached())(hCurrentProcessWindow, lpOverlayWindowClass, sizeof(lpOverlayWindowClass)) == 0)
        return FALSE;

    if (strcmp(lpCurrentWindowUsedClass, lpCurrentWindowClass) != 0 && strcmp(lpCurrentWindowUsedClass, lpOverlayWindowClass) != 0)
    {
        SetWindowLong(hCurrentProcessWindow, GWL_EXSTYLE, UI::dwWindowStyles);
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Move the overlay window on top of the target window
 * @param hCurrentProcessWindow Handle of the overlay window
 */
void UI::MoveWindow(const HWND hCurrentProcessWindow)
{
    if (hTargetWindow == nullptr)
        return;

    RECT client;
    GetClientRect(hTargetWindow, &client);
    MapWindowPoints(hTargetWindow, NULL, (LPPOINT)&client, 2);
    if (!EqualRect(&targetClient, &client))
    {
        targetClient = client;

        SetWindowPos(hCurrentProcessWindow, nullptr, client.left + Config::iOffsetLeft, client.top + Config::iOffsetTop, client.right - client.left - Config::iOffsetLeft - Config::iOffsetRight, client.bottom - client.top - Config::iOffsetTop - Config::iOffsetBottom, SWP_SHOWWINDOW);
    }
}