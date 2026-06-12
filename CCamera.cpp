#include "stdafx.h"

#include "CCamera.h"

CCamera::CCamera() :
	X(1.0f, 0.0f, 0.0f),
	Y(0.0f, 1.0f, 0.0f),
	Z(0.0f, 0.0f, 1.0f),
	Reference(0.0f),
	Position(0.0f, 0.0f, 5.0f),
	Bin(BiasMatrixInverse())
{
}

void CCamera::CalculateRayMatrix()
{
	Vin[0] = X.x; Vin[4] = Y.x; Vin[8] = Z.x;
	Vin[1] = X.y; Vin[5] = Y.y; Vin[9] = Z.y;
	Vin[2] = X.z; Vin[6] = Y.z; Vin[10] = Z.z;

	RayMatrix = Vin * Pin * Bin * VPin;
}

void CCamera::LookAt(const Vector3& reference, const Vector3& position, bool RotateAroundReference)
{
	Reference = reference;
	Position = position;

	Z = normalize(Position - Reference);
	X = normalize(cross(Vector3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);

	if (!RotateAroundReference)
	{
		Reference = Position;
		Position += Z * 0.05f;
	}

	CalculateRayMatrix();
}

bool CCamera::OnKeyDown(UINT nChar)
{
	float distance = 0.125f;
	if (GetKeyState(VK_CONTROL) & 0x80) distance *= 0.5f;
	if (GetKeyState(VK_SHIFT) & 0x80) distance *= 2.0f;

	Vector3 Up(0.0f, 1.0f, 0.0f);
	Vector3 Right = X;
	Vector3 Forward = cross(Up, Right);

	Up *= distance;
	Right *= distance;
	Forward *= distance;

	Vector3 Movement;

	if (nChar == 'W') Movement += Forward;
	if (nChar == 'S') Movement -= Forward;
	if (nChar == 'A') Movement -= Right;
	if (nChar == 'D') Movement += Right;
	if (nChar == 'R') Movement += Up;
	if (nChar == 'F') Movement -= Up;

	Reference += Movement;
	Position += Movement;

	return Movement.x != 0.0f || Movement.y != 0.0f || Movement.z != 0.0f;
}

void CCamera::OnMouseMove(int dx, int dy)
{
	constexpr float sensitivity = 0.25f;
	float hangle = (float)dx * sensitivity;
	float vangle = (float)dy * sensitivity;

	Position -= Reference;

	Y = rotate(Y, vangle, X);
	Z = rotate(Z, vangle, X);

	if (Y.y < 0.0f)
	{
		Z = Vector3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
		Y = cross(Z, X);
	}

	X = rotate(X, hangle, Vector3(0.0f, 1.0f, 0.0f));
	Y = rotate(Y, hangle, Vector3(0.0f, 1.0f, 0.0f));
	Z = rotate(Z, hangle, Vector3(0.0f, 1.0f, 0.0f));

	Position = Reference + Z * length(Position);

	CalculateRayMatrix();
}

void CCamera::OnMouseWheel(short zDelta)
{
	Position -= Reference;

	float distance = length(Position);
	if (zDelta < 0 && distance < 500.0f)
	{
		Position += Position * 0.1f;
	}
	else if (zDelta > 0 && distance > 0.05f)
	{
		Position -= Position * 0.1f;
	}

	Position += Reference;
}

CCamera Camera;