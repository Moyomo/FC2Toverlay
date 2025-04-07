#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "pch.hpp"

extern ImGuiKey ImGui_ImplWin32_KeyEventToImGuiKey(WPARAM wParam, LPARAM lParam);

class Config
{
private:
    static bool lastConnectionStatus;

public:
    static bool bStreamProof;
    static bool bAutostart;
    static bool bDebug;
    static int iTargetFPS;
    static std::chrono::microseconds targetFrametime;
    static bool bCreateOverlay;
    static int iRandomOffsetMin;
    static int iRandomOffsetMax;
    static int iOffsetLeft;
    static int iOffsetTop;
    static int iOffsetRight;
    static int iOffsetBottom;
    static int iQuitKeycode;

    static bool IsConstellationConnected();
    static void GetConfig();
    static void SaveConfig();
    static void SetRandomDimensions();
    static int ImGuiKeyToVirtualKeycode(ImGuiKey key);
};

#endif