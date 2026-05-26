#pragma once

#include <windows.h>

#include "glmath.h"

class CCamera
{
public:
	vec3 X, Y, Z, Reference, Position;
	mat4x4 Vin, Pin, Bin, VPin, RayMatrix;
	CCamera();
	~CCamera();

	void CalculateRayMatrix();
	void LookAt(const vec3& Reference, const vec3& Position, bool RotateAroundReference = false);
	bool OnKeyDown(UINT nChar);
	void OnMouseMove(int dx, int dy);
	void OnMouseWheel(short zDelta);
};