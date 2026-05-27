#pragma once

#include "glmath.h"

class CTexture;

class CSphere
{
public:
	float Radius, Radius2, ODRadius, Reflection, Refraction, Eta, ODEta;
	Vector3 Position, Color;
	CTexture* Texture;

public:
	CSphere();
	CSphere(const Vector3& Position, float Radius, const Vector3& Color, CTexture* Texture = NULL, float Reflection = 0.0f, float Refraction = 0.0f, float Eta = 1.0f);

	bool Intersect(Vector3& Origin, const Vector3& Ray, float MaxDistance, float& Distance, Vector3& Point);
	bool Intersect(Vector3& Origin, const Vector3& Ray, float MaxDistance, float& Distance);
	bool Intersect(Vector3& Origin, const Vector3& Ray, float MaxDistance);
};