#include "config.hpp"

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
}

/**
 * @brief Send the current config values to Constellation and save them to the cloud
 */
void Config::SaveConfig()
{
    std::string jsonValue = std::format("{{ \"streamproof\": {0:s}, \"target_framerate\": {1:d}, \"autostart\": {2:s}, \"debug_mode\": {3:s}, \"random_dimensions_min\": {4:d}, \"random_dimensions_max\": {5:d} }}", bStreamProof, iTargetFPS, bAutostart, bDebug, iRandomOffsetMin, iRandomOffsetMax);
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