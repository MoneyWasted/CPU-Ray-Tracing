#include "stdafx.h"

#include "CSphere.h"

#include "CTexture.h"

CSphere::CSphere()
{
}

CSphere::CSphere(
	const Vector3& Position,
	float Radius,
	const Vector3& Color,
	CTexture* Texture,
	float Reflection,
	float Refraction,
	float Eta
) :
	Radius(Radius),
	Reflection(Reflection),
	Refraction(Refraction),
	Eta(Eta),
	Position(Position),
	Color(Color),
	Texture(Texture)
{
	Radius2 = Radius * Radius;
	ODRadius = 1.0f / Radius;
	ODEta = 1.0f / Eta;
}

bool CSphere::Intersect(const Vector3& Origin, const Vector3& Ray, float MaxDistance, float& Distance, Vector3& Point)
{
	Vector3 L = Position - Origin;

	float LdotR = dot(L, Ray);
	if (LdotR <= 0.0f) return false;

	float D2 = length2(L) - LdotR * LdotR;
	if (D2 >= Radius2) return false;

	Distance = LdotR - sqrtf(Radius2 - D2);
	if (Distance < 0.0f || Distance >= MaxDistance) return false;

	Point = Ray * Distance + Origin;
	return true;
}

bool CSphere::Intersect(const Vector3& Origin, const Vector3& Ray, float MaxDistance, float& Distance)
{
	Vector3 Point;
	return Intersect(Origin, Ray, MaxDistance, Distance, Point);
}

bool CSphere::Intersect(const Vector3& Origin, const Vector3& Ray, float MaxDistance)
{
	float Distance;
	Vector3 Point;
	return Intersect(Origin, Ray, MaxDistance, Distance, Point);
}