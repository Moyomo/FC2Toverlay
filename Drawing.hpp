#ifndef DRAWING_HPP
#define DRAWING_HPP

#include "pch.hpp"

class Drawing
{
private:
    static std::chrono::steady_clock::time_point errorTime;
    static bool bDrawSettings;
    static ImGuiID lastKeyLabelID;

public:
    static ImGuiKey quitKey;
    static bool IsSettingsWindowActive();
    static void DrawSettings();
    static void DrawOverlay();
    static int FilterChars(ImGuiInputTextCallbackData* data);
    static void HelpMarker(const char* desc);
    static bool Hotkey(const char* label, ImGuiKey& key);
};

#endif