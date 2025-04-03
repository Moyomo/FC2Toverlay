#include "UI.hpp"
#include "Config.hpp"

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    // create mutex to make sure only one instance can be open at a time
    HANDLE mutex = CreateMutexA(NULL, TRUE, "FC2Toverlay");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBox(
            NULL,
            (LPCWSTR)L"There is another instance of the overlay already running.\nClose the other instance before launching the overlay again.",
            (LPCWSTR)L"Overlay already open",
            MB_ICONWARNING | MB_OK
        );
        return -1;
    }

    // for timeBeginPeriod
    #pragma comment(lib, "winmm.lib")
    // since Windows 10 version 2004 this doesn't affect the global timer resolution anymore
    timeBeginPeriod(1);

    // create settings window
    UI::RenderSettingsWindow();

    // create overlay window
    if (Config::bCreateOverlay)
        UI::RenderOverlay();

    // close mutex so new instances can get launched
    CloseHandle(mutex);

    return 0;
}