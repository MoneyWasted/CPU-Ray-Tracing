#include "stdafx.h"

#include "CRayTracer.h"
#include "CQuad.h"
#include "CSphere.h"
#include "CLight.h"

float TDRM = 2.0f / (float)RAND_MAX;
float ODRM = 1.0f / (float)RAND_MAX;

bool CRayTracer::Shadow(void* Object, Vector3& Point, Vector3& LightDirection, float LightDistance)
{
	for (CSphere* Sphere = Spheres; Sphere < LastSphere; Sphere++)
	{
		if (Sphere == Object) continue;

		if (Sphere->Intersect(Point, LightDirection, LightDistance))
		{
			return true;
		}
	}

	for (CQuad* Quad = Quads; Quad < LastQuad; Quad++)
	{
		if (Quad == Object) continue;

		if (Quad->Intersect(Point, LightDirection, LightDistance))
		{
			return true;
		}
	}

	return false;
}

Vector3 CRayTracer::LightIntensity(void* Object, Vector3& Point, Vector3& Normal, Vector3& LightPosition, CLight* Light, float AO)
{
	Vector3 LightDirection = LightPosition - Point;

	float LightDistance2 = length2(LightDirection);
	float LightDistance = sqrtf(LightDistance2);

	LightDirection *= 1.0f / LightDistance;

	float Attenuation = Light->QuadraticAttenuation * LightDistance2 + Light->LinearAttenuation * LightDistance + Light->ConstantAttenuation;

	float NdotLD = dot(Normal, LightDirection);

	if (NdotLD > 0.0f)
	{
		if (Light->Sphere)
		{
			if (Shadow(Object, Point, LightDirection, LightDistance) == false)
			{
				return Light->Sphere->Color * ((Light->Ambient * AO + Light->Diffuse * NdotLD) / Attenuation);
			}
		}
		else
		{
			float LNdotLD = -dot(Light->Quad->N, LightDirection);

			if (LNdotLD > 0.0f)
			{
				if (Shadow(Object, Point, LightDirection, LightDistance) == false)
				{
					return Light->Quad->Color * ((Light->Ambient * AO + Light->Diffuse * NdotLD * LNdotLD) / Attenuation);
				}
			}
		}
	}

	return (Light->Sphere ? Light->Sphere->Color : Light->Quad->Color) * (Light->Ambient * AO / Attenuation);
}

float CRayTracer::AmbientOcclusionFactor(void* Object, Vector3& Point, Vector3& Normal)
{
	float AO = 0.0f;

	for (int i = 0; i < GISamples; i++)
	{
		Vector3 RandomRay = normalize(Vector3(TDRM * (float)rand() - 1.0f, TDRM * (float)rand() - 1.0f, TDRM * (float)rand() - 1.0f));

		float NdotRR = dot(Normal, RandomRay);

		if (NdotRR < 0.0f)
		{
			RandomRay = -RandomRay;
			NdotRR = -NdotRR;
		}

		float Distance = 1048576.0f, TestDistance;

		for (CSphere* Sphere = Spheres; Sphere < LastSphere; Sphere++)
		{
			if (Sphere == Object) continue;

			if (Sphere->Intersect(Point, RandomRay, Distance, TestDistance))
			{
				Distance = TestDistance;
			}
		}

		for (CQuad* Quad = Quads; Quad < LastQuad; Quad++)
		{
			if (Quad == Object) continue;

			if (Quad->Intersect(Point, RandomRay, Distance, TestDistance))
			{
				Distance = TestDistance;
			}
		}

		AO += NdotRR / (1.0f + Distance * Distance);
	}

	return 1.0f - AO * ODGISamplesMAmbientOcclusionIntensity;
}

void CRayTracer::IlluminatePoint(void* Object, Vector3& Point, Vector3& Normal, Vector3& Color)
{
	float AO = 1.0f;

	if (AmbientOcclusion)
	{
		AO = AmbientOcclusionFactor(Object, Point, Normal);
	}

	if (LightsCount == 0)
	{
		float NdotCD = dot(Normal, normalize(Camera.Position - Point));

		if (NdotCD > 0.0f)
		{
			Color *= 0.5f * (AO + NdotCD);
		}
		else
		{
			Color *= 0.5f * AO;
		}
	}
	else if (SoftShadows == false)
	{
		Vector3 LightsIntensitiesSum;

		for (CLight* Light = Lights; Light < LastLight; Light++)
		{
			LightsIntensitiesSum += LightIntensity(Object, Point, Normal, Light->Sphere ? Light->Sphere->Position : Light->Quad->m, Light, AO);
		}

		Color *= LightsIntensitiesSum;
	}
	else
	{
		Vector3 LightsIntensitiesSum;

		for (CLight* Light = Lights; Light < LastLight; Light++)
		{
			if (Light->Sphere)
			{
				for (int i = 0; i < GISamples; i++)
				{
					Vector3 RandomRay = /*normalize(*/Vector3(TDRM * (float)rand() - 1.0f, TDRM * (float)rand() - 1.0f, TDRM * (float)rand() - 1.0f)/*)*/;

					Vector3 RandomLightPosition = RandomRay * Light->Sphere->Radius + Light->Sphere->Position;

					LightsIntensitiesSum += LightIntensity(Object, Point, Normal, RandomLightPosition, Light, AO);
				}
			}
			else
			{
				for (int i = 0; i < GISamples; i++)
				{
					float s = ODRM * (float)rand();
					float t = ODRM * (float)rand();

					Vector3 RandomLightPosition = Light->Quad->ab * s + Light->Quad->ad * t + Light->Quad->a;

					LightsIntensitiesSum += LightIntensity(Object, Point, Normal, RandomLightPosition, Light, AO);
				}
			}
		}

		Color *= LightsIntensitiesSum * ODGISamples;
	}
}