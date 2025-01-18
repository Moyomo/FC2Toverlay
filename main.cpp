#include "UI.h"
#include "fc2.hpp"

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    #pragma comment(lib, "winmm.lib") // for timeBeginPeriod
    // Since Windows 10 version 2004 this doesn't affect the global timer resolution anymore
    timeBeginPeriod(1);

    // check if Constellation4 is running
    FC2_TEAM_ERROR_CODES error_code = fc2::get_error();
    if (error_code != FC2_TEAM_ERROR_NO_ERROR)
    {
        MessageBox(
            NULL,
            (LPCWSTR)L"Constellation4 doesn't appear to be open.\nLaunch Constellation4 and restart the overlay.",
            (LPCWSTR)L"Solution not found",
            MB_ICONWARNING | MB_OK
        );
        return -1;
    }

    // check if Constellation4 is calibrated
    if (!UI::SetTargetWindow())
    {
        MessageBox(
            NULL,
            (LPCWSTR)L"Constellation4 doesn't appear to be calibrated to a game.\nCalibrate Constellation4 and restart the overlay.",
            (LPCWSTR)L"Solution not calibrated",
            MB_ICONWARNING | MB_OK
        );
        return -1;
    }

    // create overlay
    UI::Render();

    return 0;
}