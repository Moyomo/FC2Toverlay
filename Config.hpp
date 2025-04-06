#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "pch.hpp"

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

    static bool IsConstellationConnected();
    static void GetConfig();
    static void SaveConfig();
    static void SetRandomDimensions();
};

#endif