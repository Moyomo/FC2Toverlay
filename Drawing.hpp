#ifndef DRAWING_HPP
#define DRAWING_HPP

#include "pch.hpp"

class Drawing
{
private:
    static LPCSTR lpWindowName;
    static ImVec2 vWindowSize;

public:
    static void Draw(BOOL bDebug);
};

#endif