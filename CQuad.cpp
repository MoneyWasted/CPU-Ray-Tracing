#include "stdafx.h"

#include "CQuad.h"

#include "CTexture.h"

CQuad::CQuad()
{
}

CQuad::CQuad(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d, const Vector3& Color, CTexture* Texture, float Reflection, float Refraction, float Eta) : a(a), b(b), c(c), d(d), N(N), D(D), Color(Color), Texture(Texture), Reflection(Reflection), Refraction(Refraction), Eta(Eta)
{
	ab = b - a;
	ad = d - a;
	m = (a + b + c + d) / 4.0f;

	T = normalize(b - a);
	N = normalize(cross(b - a, c - a));
	B = cross(N, T);
	O = Vector3(dot(T, a), dot(B, a), dot(N, a));

	D = -dot(N, a);
	ODEta = 1.0f / Eta;

	N1 = normalize(cross(N, b - a));
	D1 = -dot(N1, a);

	N2 = normalize(cross(N, c - b));
	D2 = -dot(N2, b);

	N3 = normalize(cross(N, d - c));
	D3 = -dot(N3, c);

	N4 = normalize(cross(N, a - d));
	D4 = -dot(N4, d);
}

bool CQuad::Inside(const Vector3& Point)
{
	if (dot(N1, Point) + D1 < 0.0f) return false;
	if (dot(N2, Point) + D2 < 0.0f) return false;
	if (dot(N3, Point) + D3 < 0.0f) return false;
	if (dot(N4, Point) + D4 < 0.0f) return false;

	return true;
}

bool CQuad::Intersect(Vector3& Origin, const Vector3& Ray, float MaxDistance, float& Distance, Vector3& Point)
{
	float NdotR = -dot(N, Ray);

	if (NdotR > 0.0f)
	{
		Distance = (dot(N, Origin) + D) / NdotR;

		if (Distance >= 0.0f && Distance < MaxDistance)
		{
			Point = Ray * Distance + Origin;

			return Inside(Point);
		}
	}

	return false;
}

bool CQuad::Intersect(Vector3& Origin, const Vector3& Ray, float MaxDistance, float& Distance)
{
	float NdotR = -dot(N, Ray);

	if (NdotR > 0.0f)
	{
		Distance = (dot(N, Origin) + D) / NdotR;

		if (Distance >= 0.0f && Distance < MaxDistance)
		{
			return Inside(Ray * Distance + Origin);
		}
	}

	return false;
}

bool CQuad::Intersect(Vector3& Origin, const Vector3& Ray, float MaxDistance)
{
	float NdotR = -dot(N, Ray);

	if (NdotR > 0.0f)
	{
		float Distance = (dot(N, Origin) + D) / NdotR;

		if (Distance >= 0.0f && Distance < MaxDistance)
		{
			return Inside(Ray * Distance + Origin);
		}
	}

	return false;
}