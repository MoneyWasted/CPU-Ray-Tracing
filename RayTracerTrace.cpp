#include "stdafx.h"

#include "CRayTracer.h"
#include "CQuad.h"
#include "CSphere.h"
#include "CLight.h"
#include "CTexture.h"

static const float M_1_PI_2 = (float)M_1_PI * 0.5f;

static constexpr UINT MaxRayDepth = 8;

Vector3 CRayTracer::RayTrace(const Vector3& Origin, const Vector3& Ray, UINT Depth, void* Object)
{
	float distance = 1048576.0f;
	float testDistance = 0.0f;
	Vector3 hitPoint;
	Vector3 testPoint;
	CSphere* hitSphere = nullptr;
	CQuad* hitQuad = nullptr;
	CLight* hitLight = nullptr;
	Vector3 color;

	if (Depth >= MaxRayDepth) return color;

	for (CSphere* Sphere = Spheres; Sphere < LastSphere; Sphere++)
	{
		if (Sphere == Object) continue;

		if (Sphere->Intersect(Origin, Ray, distance, testDistance, testPoint))
		{
			hitPoint = testPoint;
			distance = testDistance;
			hitSphere = Sphere;
		}
	}

	for (CQuad* Quad = Quads; Quad < LastQuad; Quad++)
	{
		if (Quad == Object) continue;

		if (Quad->Intersect(Origin, Ray, distance, testDistance, testPoint))
		{
			hitPoint = testPoint;
			distance = testDistance;
			hitQuad = Quad;
		}
	}

	for (CLight* Light = Lights; Light < LastLight; Light++)
	{
		if (Light->Sphere)
		{
			if (Light->Sphere->Intersect(Origin, Ray, distance, testDistance, testPoint))
			{
				hitPoint = testPoint;
				distance = testDistance;
				hitLight = Light;
			}
		}
		else
		{
			if (Light->Quad->Intersect(Origin, Ray, distance, testDistance, testPoint))
			{
				hitPoint = testPoint;
				distance = testDistance;
				hitLight = Light;
			}
		}
	}

	if (hitLight)
	{
		color = hitLight->Sphere ? hitLight->Sphere->Color : hitLight->Quad->Color;
	}
	else if (hitQuad)
	{
		color = hitQuad->Color;

		if (Textures && hitQuad->Texture)
		{
			float s = dot(hitQuad->T, hitPoint) - hitQuad->O.x;
			float t = dot(hitQuad->B, hitPoint) - hitQuad->O.y;

			color *= hitQuad->Texture->GetColorBilinear(s, t);
		}

		IlluminatePoint(hitQuad, hitPoint, hitQuad->N, color);

		if (hitQuad->Reflection > 0.0f)
		{
			Vector3 ReflectedRay = reflect(Ray, hitQuad->N);

			color = mix(color, RayTrace(hitPoint, ReflectedRay, Depth + 1, hitQuad), hitQuad->Reflection);
		}
	}
	else if (hitSphere)
	{
		color = hitSphere->Color;

		Vector3 Normal = (hitPoint - hitSphere->Position) * hitSphere->ODRadius;

		if (Textures && hitSphere->Texture)
		{
			float s = atan2f(Normal.x, Normal.z) * M_1_PI_2 + 0.5f;
			float t = asinf(Normal.y < -1.0f ? -1.0f : Normal.y > 1.0f ? 1.0f : Normal.y) * (float)M_1_PI + 0.5f;

			color *= hitSphere->Texture->GetColorBilinear(s, t);
		}

		IlluminatePoint(hitSphere, hitPoint, Normal, color);

		if (hitSphere->Refraction > 0.0f)
		{
			Vector3 RefractedRay = refract(Ray, Normal, hitSphere->ODEta);

			Vector3 L = hitSphere->Position - hitPoint;
			float LdotRR = dot(L, RefractedRay);
			float D2 = length2(L) - LdotRR * LdotRR;
			float Distance = LdotRR + sqrtf(hitSphere->Radius2 - D2);

			Vector3 NewPoint = RefractedRay * Distance + hitPoint;

			Vector3 NewNormal = (hitSphere->Position - NewPoint) * hitSphere->ODRadius;

			RefractedRay = refract(RefractedRay, NewNormal, hitSphere->Eta);

			color = mix(color, RayTrace(NewPoint, RefractedRay, Depth + 1, hitSphere), hitSphere->Refraction);
		}

		if (hitSphere->Reflection > 0.0f)
		{
			Vector3 ReflectedRay = reflect(Ray, Normal);

			color = mix(color, RayTrace(hitPoint, ReflectedRay, Depth + 1, hitSphere), hitSphere->Reflection);
		}
	}

	return color;
}