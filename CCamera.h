#pragma once

#include <windows.h>

#include "glmath.h"

class CCamera
{
public:
	Vector3 X, Y, Z, Reference, Position;
	Matrix4x4 Vin, Pin, Bin, VPin, RayMatrix;
	CCamera();
	~CCamera();

	void CalculateRayMatrix();
	void LookAt(const Vector3& Reference, const Vector3& Position, bool RotateAroundReference = false);
	bool OnKeyDown(UINT nChar);
	void OnMouseMove(int dx, int dy);
	void OnMouseWheel(short zDelta);
};