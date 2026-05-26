#pragma once

#include "glmath.h"

class CTexture;

class CQuad
{
public:
	float Reflection, Refraction, Eta, ODEta, D, D1, D2, D3, D4;
	vec3 a, b, c, d, Color, ab, ad, m, T, B, N, O, N1, N2, N3, N4;
	CTexture* Texture;

public:
	CQuad();
	CQuad(const vec3& a, const vec3& b, const vec3& c, const vec3& d, const vec3& Color, CTexture* Texture = NULL, float Reflection = 0.0f, float Refraction = 0.0f, float Eta = 1.0f);

	bool Inside(const vec3& Point);
	bool Intersect(vec3& Origin, const vec3& Ray, float MaxDistance, float& Distance, vec3& Point);
	bool Intersect(vec3& Origin, const vec3& Ray, float MaxDistance, float& Distance);
	bool Intersect(vec3& Origin, const vec3& Ray, float MaxDistance);
};