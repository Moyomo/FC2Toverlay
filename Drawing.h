#ifndef DRAWING_H
#define DRAWING_H

#include "pch.h"

class Drawing
{
private:
	static LPCSTR lpWindowName;
	static ImVec2 vWindowSize;

public:
	static void Draw(BOOL bDebug);
};

#endif