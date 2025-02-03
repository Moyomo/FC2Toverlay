#include "config.hpp"

// define default values
bool Config::bStreamProof = true;
bool Config::bAutostart = false;
bool Config::bDebug = false;
int Config::iTargetFPS = 250;
std::chrono::microseconds Config::targetFrametime{ 4000 };
bool Config::lastConnectionStatus = false;
bool Config::bCreateOverlay = false;

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
}

/**
 * @brief Send the current config values to Constellation and save them to the cloud
 */
void Config::SaveConfig()
{
    std::string jsonValue = std::format("{{ \"streamproof\": {0:s}, \"target_framerate\": {1:d}, \"autostart\": {2:s}, \"debug_mode\": {3:s} }}", bStreamProof, iTargetFPS, bAutostart, bDebug);
    fc2::call("directx_overlay_save", jsonValue);
}