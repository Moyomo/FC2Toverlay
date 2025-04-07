#include "Config.hpp"
#include "Drawing.hpp"

// define default values
bool Config::bStreamProof = true;
bool Config::bAutostart = false;
bool Config::bDebug = false;
int Config::iTargetFPS = 250;
std::chrono::microseconds Config::targetFrametime{ 4000 };
bool Config::lastConnectionStatus = false;
bool Config::bCreateOverlay = false;
int Config::iRandomOffsetMin = 0;
int Config::iRandomOffsetMax = 0;
int Config::iOffsetLeft = 0;
int Config::iOffsetTop = 0;
int Config::iOffsetRight = 0;
int Config::iOffsetBottom = 0;
int Config::iQuitKeycode = VK_END;

// random number generator
static std::random_device rd;
static std::mt19937 gen(rd());

/**
 * @brief Check if there is an active connection to Constellation
 * @return true if Constellation is connected, otherwise false
 */
bool Config::IsConstellationConnected()
{
    // call FC2T function to update the last error
    fc2::get_session();

    // get the last fc2 error
    bool connectionStatus = fc2::get_error() == FC2_TEAM_ERROR_NO_ERROR;

    // get config settings if constellation just connected
    if (connectionStatus && !lastConnectionStatus)
        GetConfig();

    // save and return the new connection status
    lastConnectionStatus = connectionStatus;
    return connectionStatus;
}

/**
 * @brief Get the saved config values from Constellation
 */
void Config::GetConfig()
{
    // check if Constellation is running
    if (fc2::get_error() != FC2_TEAM_ERROR_NO_ERROR)
        return;

    // get saved config values
    bStreamProof = fc2::call<BOOL>("directx_overlay_streamproof", FC2_LUA_TYPE_BOOLEAN);
    bAutostart = fc2::call<BOOL>("directx_overlay_autostart", FC2_LUA_TYPE_BOOLEAN);
    bDebug = fc2::call<BOOL>("directx_overlay_debug", FC2_LUA_TYPE_BOOLEAN);
    iTargetFPS = fc2::call<uint32_t>("directx_overlay_target_fps", FC2_LUA_TYPE_INT);
    iTargetFPS = std::min(1000, std::max(iTargetFPS, 0));
    targetFrametime = std::chrono::microseconds(iTargetFPS == 0 ? 1 : 1000000 / iTargetFPS);
    iRandomOffsetMin = fc2::call<int>("directx_overlay_random_min", FC2_LUA_TYPE_INT);
    iRandomOffsetMax = fc2::call<int>("directx_overlay_random_max", FC2_LUA_TYPE_INT);
    iQuitKeycode = fc2::call<int>("directx_overlay_quit_key", FC2_LUA_TYPE_INT);

    // set button text for custom key
    Drawing::quitKey = ImGui_ImplWin32_KeyEventToImGuiKey(iQuitKeycode, 0);
}

/**
 * @brief Send the current config values to Constellation and save them to the cloud
 */
void Config::SaveConfig()
{
    std::string jsonValue = std::format("{{ \"streamproof\": {0:s}, \"target_framerate\": {1:d}, \"autostart\": {2:s}, \"debug_mode\": {3:s}, \"random_dimensions_min\": {4:d}, \"random_dimensions_max\": {5:d}, \"quit_key\": {6:d} }}", bStreamProof, iTargetFPS, bAutostart, bDebug, iRandomOffsetMin, iRandomOffsetMax, iQuitKeycode);
    fc2::call("directx_overlay_save", jsonValue);
}

/**
 * @brief Generate random offsets for the overlay size
 */
void Config::SetRandomDimensions()
{
    // get the range for the random offsets
    int iRandomMin = std::max(-100, std::min(iRandomOffsetMin, iRandomOffsetMax));
    int iRandomMax = std::min(100, std::max(iRandomOffsetMin, iRandomOffsetMax));

    // set range for random number generator
    std::uniform_int_distribution<> distrib(iRandomMin, iRandomMax);

    // generate separate random offsets for each side of the overlay
    iOffsetLeft = distrib(gen);
    iOffsetTop = distrib(gen);
    iOffsetRight = distrib(gen);
    iOffsetBottom = distrib(gen);
}

/**
 * @brief Map ImGuiKey_xxx to VK_xxx. Opposite of ImGui_ImplWin32_KeyEventToImGuiKey
 * @param key ImGuiKey that should get translated to a virtual key code
 * @return virtual key code of the given ImGuiKey
 */
int Config::ImGuiKeyToVirtualKeycode(ImGuiKey key)
{
    switch (key)
    {
        case ImGuiKey_Tab: return VK_TAB;
        case ImGuiKey_LeftArrow: return VK_LEFT;
        case ImGuiKey_RightArrow: return VK_RIGHT;
        case ImGuiKey_UpArrow: return VK_UP;
        case ImGuiKey_DownArrow: return VK_DOWN;
        case ImGuiKey_PageUp: return VK_PRIOR;
        case ImGuiKey_PageDown: return VK_NEXT;
        case ImGuiKey_Home: return VK_HOME;
        case ImGuiKey_End: return VK_END;
        case ImGuiKey_Insert: return VK_INSERT;
        case ImGuiKey_Delete: return VK_DELETE;
        case ImGuiKey_Backspace: return VK_BACK;
        case ImGuiKey_Space: return VK_SPACE;
        case ImGuiKey_Enter: return VK_RETURN;
        case ImGuiKey_Escape: return VK_ESCAPE;
        case ImGuiKey_LeftCtrl: return VK_LCONTROL;
        case ImGuiKey_LeftShift: return VK_LSHIFT;
        case ImGuiKey_LeftAlt: return VK_LMENU;
        case ImGuiKey_LeftSuper: return VK_LWIN;
        case ImGuiKey_RightCtrl: return VK_RCONTROL;
        case ImGuiKey_RightShift: return VK_RSHIFT;
        case ImGuiKey_RightAlt: return VK_RMENU;
        case ImGuiKey_RightSuper: return VK_RWIN;
        case ImGuiKey_Menu: return VK_APPS;
        case ImGuiKey_0: return 0x30;
        case ImGuiKey_1: return 0x31;
        case ImGuiKey_2: return 0x32;
        case ImGuiKey_3: return 0x33;
        case ImGuiKey_4: return 0x34;
        case ImGuiKey_5: return 0x35;
        case ImGuiKey_6: return 0x36;
        case ImGuiKey_7: return 0x37;
        case ImGuiKey_8: return 0x38;
        case ImGuiKey_9: return 0x39;
        case ImGuiKey_A: return 0x41;
        case ImGuiKey_B: return 0x42;
        case ImGuiKey_C: return 0x43;
        case ImGuiKey_D: return 0x44;
        case ImGuiKey_E: return 0x45;
        case ImGuiKey_F: return 0x46;
        case ImGuiKey_G: return 0x47;
        case ImGuiKey_H: return 0x48;
        case ImGuiKey_I: return 0x49;
        case ImGuiKey_J: return 0x4A;
        case ImGuiKey_K: return 0x4B;
        case ImGuiKey_L: return 0x4C;
        case ImGuiKey_M: return 0x4D;
        case ImGuiKey_N: return 0x4E;
        case ImGuiKey_O: return 0x4F;
        case ImGuiKey_P: return 0x50;
        case ImGuiKey_Q: return 0x51;
        case ImGuiKey_R: return 0x52;
        case ImGuiKey_S: return 0x53;
        case ImGuiKey_T: return 0x54;
        case ImGuiKey_U: return 0x55;
        case ImGuiKey_V: return 0x56;
        case ImGuiKey_W: return 0x57;
        case ImGuiKey_X: return 0x58;
        case ImGuiKey_Y: return 0x59;
        case ImGuiKey_Z: return 0x5A;
        case ImGuiKey_F1: return VK_F1;
        case ImGuiKey_F2: return VK_F2;
        case ImGuiKey_F3: return VK_F3;
        case ImGuiKey_F4: return VK_F4;
        case ImGuiKey_F5: return VK_F5;
        case ImGuiKey_F6: return VK_F6;
        case ImGuiKey_F7: return VK_F7;
        case ImGuiKey_F8: return VK_F8;
        case ImGuiKey_F9: return VK_F9;
        case ImGuiKey_F10: return VK_F10;
        case ImGuiKey_F11: return VK_F11;
        case ImGuiKey_F12: return VK_F12;
        case ImGuiKey_F13: return VK_F13;
        case ImGuiKey_F14: return VK_F14;
        case ImGuiKey_F15: return VK_F15;
        case ImGuiKey_F16: return VK_F16;
        case ImGuiKey_F17: return VK_F17;
        case ImGuiKey_F18: return VK_F18;
        case ImGuiKey_F19: return VK_F19;
        case ImGuiKey_F20: return VK_F20;
        case ImGuiKey_F21: return VK_F21;
        case ImGuiKey_F22: return VK_F22;
        case ImGuiKey_F23: return VK_F23;
        case ImGuiKey_F24: return VK_F24;
        case ImGuiKey_Apostrophe: return VK_OEM_7;
        case ImGuiKey_Comma: return VK_OEM_COMMA;
        case ImGuiKey_Minus: return VK_OEM_MINUS;
        case ImGuiKey_Period: return VK_OEM_PERIOD;
        case ImGuiKey_Slash: return VK_OEM_2;
        case ImGuiKey_Semicolon: return VK_OEM_1;
        case ImGuiKey_Equal: return VK_OEM_PLUS;
        case ImGuiKey_LeftBracket: return VK_OEM_4;
        case ImGuiKey_Backslash: return VK_OEM_5;
        case ImGuiKey_RightBracket: return VK_OEM_6;
        case ImGuiKey_GraveAccent: return VK_OEM_3;
        case ImGuiKey_CapsLock: return VK_CAPITAL;
        case ImGuiKey_ScrollLock: return VK_SCROLL;
        case ImGuiKey_NumLock: return VK_NUMLOCK;
        case ImGuiKey_PrintScreen: return VK_SNAPSHOT;
        case ImGuiKey_Pause: return VK_PAUSE;
        case ImGuiKey_Keypad0: return VK_NUMPAD0;
        case ImGuiKey_Keypad1: return VK_NUMPAD1;
        case ImGuiKey_Keypad2: return VK_NUMPAD2;
        case ImGuiKey_Keypad3: return VK_NUMPAD3;
        case ImGuiKey_Keypad4: return VK_NUMPAD4;
        case ImGuiKey_Keypad5: return VK_NUMPAD5;
        case ImGuiKey_Keypad6: return VK_NUMPAD6;
        case ImGuiKey_Keypad7: return VK_NUMPAD7;
        case ImGuiKey_Keypad8: return VK_NUMPAD8;
        case ImGuiKey_Keypad9: return VK_NUMPAD9;
        case ImGuiKey_KeypadDecimal: return VK_DECIMAL;
        case ImGuiKey_KeypadDivide: return VK_DIVIDE;
        case ImGuiKey_KeypadMultiply: return VK_MULTIPLY;
        case ImGuiKey_KeypadSubtract: return VK_SUBTRACT;
        case ImGuiKey_KeypadAdd: return VK_ADD;
        case ImGuiKey_AppBack: return VK_BROWSER_BACK;
        case ImGuiKey_AppForward: return VK_BROWSER_FORWARD;
        case ImGuiKey_MouseLeft: return VK_LBUTTON;
        case ImGuiKey_MouseRight: return VK_RBUTTON;
        case ImGuiKey_MouseMiddle: return VK_MBUTTON;
        case ImGuiKey_MouseX1: return VK_XBUTTON1;
        case ImGuiKey_MouseX2: return VK_XBUTTON2;
        default: return 0;
    }
}