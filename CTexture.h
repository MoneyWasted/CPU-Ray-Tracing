#pragma once

#include <vector>
#include <windows.h>

#include "glmath.h"

class CTexture
{
private:
	std::vector<BYTE> Data;
	int Width, Height;

public:
	CTexture();
	~CTexture();

	bool CreateTexture2D(const char* Texture2DFileName);
	Vector3 GetColorNearest(float s, float t);
	Vector3 GetColorBilinear(float s, float t);
	void Destroy();
};