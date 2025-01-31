#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "pch.hpp"

class Config
{
private:
    static bool lastConnectionStatus;

public:
    static bool bStreamProof;
    static bool bDebug;
    static int iTargetFPS;
    static std::chrono::microseconds targetFrametime;
    static bool bCreateOverlay;

    static bool IsConstellationConnected();
    static void GetConfig();
    static void SaveConfig();
};

#endif