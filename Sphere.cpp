#include "stdafx.h"
#include "Sphere.h"
#include "Texture.h"

CSphere::CSphere()
{
}

CSphere::CSphere(const vec3& Position, float Radius, const vec3& Color, CTexture* Texture, float Reflection, float Refraction, float Eta) : Position(Position), Radius(Radius), Color(Color), Texture(Texture), Reflection(Reflection), Refraction(Refraction), Eta(Eta)
{
	Radius2 = Radius * Radius;
	ODRadius = 1.0f / Radius;
	ODEta = 1.0f / Eta;
}

bool CSphere::Intersect(vec3& Origin, const vec3& Ray, float MaxDistance, float& Distance, vec3& Point)
{
	vec3 L = Position - Origin;

	float LdotR = dot(L, Ray);

	if (LdotR > 0.0f)
	{
		float D2 = length2(L) - LdotR * LdotR;

		if (D2 < Radius2)
		{
			Distance = LdotR - sqrt(Radius2 - D2);

			if (Distance >= 0.0f && Distance < MaxDistance)
			{
				Point = Ray * Distance + Origin;

				return true;
			}
		}
	}

	return false;
}

bool CSphere::Intersect(vec3& Origin, const vec3& Ray, float MaxDistance, float& Distance)
{
	vec3 L = Position - Origin;

	float LdotR = dot(L, Ray);

	if (LdotR > 0.0f)
	{
		float D2 = length2(L) - LdotR * LdotR;

		if (D2 < Radius2)
		{
			Distance = LdotR - sqrt(Radius2 - D2);

			if (Distance >= 0.0f && Distance < MaxDistance)
			{
				return true;
			}
		}
	}

	return false;
}

bool CSphere::Intersect(vec3& Origin, const vec3& Ray, float MaxDistance)
{
	vec3 L = Position - Origin;

	float LdotR = dot(L, Ray);

	if (LdotR > 0.0f)
	{
		float D2 = length2(L) - LdotR * LdotR;

		if (D2 < Radius2)
		{
			float Distance = LdotR - sqrt(Radius2 - D2);

			if (Distance >= 0.0f && Distance < MaxDistance)
			{
				return true;
			}
		}
	}

	return false;
}