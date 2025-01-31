#include "UI.hpp"
#include "Config.hpp"

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    #pragma comment(lib, "winmm.lib") // for timeBeginPeriod
    // Since Windows 10 version 2004 this doesn't affect the global timer resolution anymore
    timeBeginPeriod(1);

    // create settings window
    UI::RenderSettingsWindow();

    // create overlay window
    if (Config::bCreateOverlay)
        UI::RenderOverlay();

    return 0;
}