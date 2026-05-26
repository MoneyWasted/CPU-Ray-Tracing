#pragma once

#include "glmath.h"

class CTexture;

class CSphere
{
public:
	float Radius, Radius2, ODRadius, Reflection, Refraction, Eta, ODEta;
	vec3 Position, Color;
	CTexture* Texture;

public:
	CSphere();
	CSphere(const vec3& Position, float Radius, const vec3& Color, CTexture* Texture = NULL, float Reflection = 0.0f, float Refraction = 0.0f, float Eta = 1.0f);

	bool Intersect(vec3& Origin, const vec3& Ray, float MaxDistance, float& Distance, vec3& Point);
	bool Intersect(vec3& Origin, const vec3& Ray, float MaxDistance, float& Distance);
	bool Intersect(vec3& Origin, const vec3& Ray, float MaxDistance);
};