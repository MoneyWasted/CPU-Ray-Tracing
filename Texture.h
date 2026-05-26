#pragma once

#include <windows.h>

#include "glmath.h"

#pragma comment(lib, "FreeImage.lib")

class CTexture
{
private:
	BYTE* Data;
	int Width, Height;

public:
	CTexture();
	~CTexture();

	bool CreateTexture2D(const char* Texture2DFileName);
	vec3 GetColorNearest(float s, float t);
	vec3 GetColorBilinear(float s, float t);
	void Destroy();
};