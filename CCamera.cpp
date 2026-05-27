#include "stdafx.h"

#include "CCamera.h"

CCamera::CCamera()
{
	X = Vector3(1.0, 0.0, 0.0);
	Y = Vector3(0.0, 1.0, 0.0);
	Z = Vector3(0.0, 0.0, 1.0);

	Reference = Vector3(0.0, 0.0, 0.0);
	Position = Vector3(0.0, 0.0, 5.0);

	Bin = BiasMatrixInverse();
}

CCamera::~CCamera()
{
}

void CCamera::CalculateRayMatrix()
{
	Vin[0] = X.x; Vin[4] = Y.x; Vin[8] = Z.x;
	Vin[1] = X.y; Vin[5] = Y.y; Vin[9] = Z.y;
	Vin[2] = X.z; Vin[6] = Y.z; Vin[10] = Z.z;

	RayMatrix = Vin * Pin * Bin * VPin;
}

void CCamera::LookAt(const Vector3& Reference, const Vector3& Position, bool RotateAroundReference)
{
	this->Reference = Reference;
	this->Position = Position;

	Z = normalize(Position - Reference);
	X = normalize(cross(Vector3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);

	if (!RotateAroundReference)
	{
		this->Reference = this->Position;
		this->Position += Z * 0.05f;
	}

	CalculateRayMatrix();
}

bool CCamera::OnKeyDown(UINT nChar)
{
	float Distance = 0.125f;

	if (GetKeyState(VK_CONTROL) & 0x80)
	{
		Distance *= 0.5f;
	}

	if (GetKeyState(VK_SHIFT) & 0x80)
	{
		Distance *= 2.0f;
	}

	Vector3 Up(0.0f, 1.0f, 0.0f);
	Vector3 Right = X;
	Vector3 Forward = cross(Up, Right);

	Up *= Distance;
	Right *= Distance;
	Forward *= Distance;

	Vector3 Movement;

	if (nChar == 'W')
	{
		Movement += Forward;
	}

	if (nChar == 'S')
	{
		Movement -= Forward;
	}

	if (nChar == 'A')
	{
		Movement -= Right;
	}

	if (nChar == 'D')
	{
		Movement += Right;
	}

	if (nChar == 'R')
	{
		Movement += Up;
	}

	if (nChar == 'F')
	{
		Movement -= Up;
	}

	Reference += Movement;
	Position += Movement;

	return Movement.x != 0.0f || Movement.y != 0.0f || Movement.z != 0.0f;
}

void CCamera::OnMouseMove(int dx, int dy)
{
	float sensitivity = 0.25f;

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

	if (zDelta < 0 && length(Position) < 500.0f)
	{
		Position += Position * 0.1f;
	}

	if (zDelta > 0 && length(Position) > 0.05f)
	{
		Position -= Position * 0.1f;
	}

	Position += Reference;
}

CCamera Camera;