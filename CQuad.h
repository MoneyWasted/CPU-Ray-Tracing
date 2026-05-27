#pragma once

#include "glmath.h"

class CTexture;

class CQuad
{
public:
	float Reflection, Refraction, Eta, ODEta, D, D1, D2, D3, D4;
	Vector3 a, b, c, d, Color, ab, ad, m, T, B, N, O, N1, N2, N3, N4;
	CTexture* Texture;

public:
	CQuad();
	CQuad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, const Vector3& Color, CTexture* Texture = NULL, float Reflection = 0.0f, float Refraction = 0.0f, float Eta = 1.0f);

	bool Inside(const Vector3& Point);
	bool Intersect(Vector3& Origin, const Vector3& Ray, float MaxDistance, float& Distance, Vector3& Point);
	bool Intersect(Vector3& Origin, const Vector3& Ray, float MaxDistance, float& Distance);
	bool Intersect(Vector3& Origin, const Vector3& Ray, float MaxDistance);
};