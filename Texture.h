#pragma once

#include <windows.h>

#include "glmath.h"

class CTexture
{
private:
	BYTE* Data;
	int Width, Height;

public:
	CTexture();
	~CTexture();

	template <typename T>
	static void SafeRelease(T*& ptr)
	{
		if (ptr != NULL)
		{
			ptr->Release();
			ptr = NULL;
		}
	}

	bool CreateTexture2D(const char* Texture2DFileName);
	vec3 GetColorNearest(float s, float t);
	vec3 GetColorBilinear(float s, float t);
	void Destroy();
};