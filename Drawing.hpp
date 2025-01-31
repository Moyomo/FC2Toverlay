#ifndef DRAWING_HPP
#define DRAWING_HPP

#include "pch.hpp"

class Drawing
{
private:
    static std::chrono::steady_clock::time_point errorTime;
    static bool bDraw;

public:
    static bool IsActive();
    static void DrawSettings();
    static void Draw();
};

#endif