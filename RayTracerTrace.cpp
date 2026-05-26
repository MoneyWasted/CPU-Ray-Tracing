#include "stdafx.h"
#include "RayTracer.h"
#include "Quad.h"
#include "Sphere.h"
#include "Light.h"
#include "Texture.h"
#include "RTData.h"

float M_1_PI_2 = (float)M_1_PI * 0.5f;

vec3 CRayTracer::RayTrace(vec3& Origin, const vec3& Ray, UINT Depth, void* Object)
{
	RTData Data;

	for (CSphere* Sphere = Spheres; Sphere < LastSphere; Sphere++)
	{
		if (Sphere == Object) continue;

		if (Sphere->Intersect(Origin, Ray, Data.Distance, Data.TestDistance, Data.TestPoint))
		{
			Data.Point = Data.TestPoint;
			Data.Distance = Data.TestDistance;
			Data.Sphere = Sphere;
		}
	}

	for (CQuad* Quad = Quads; Quad < LastQuad; Quad++)
	{
		if (Quad == Object) continue;

		if (Quad->Intersect(Origin, Ray, Data.Distance, Data.TestDistance, Data.TestPoint))
		{
			Data.Point = Data.TestPoint;
			Data.Distance = Data.TestDistance;
			Data.Quad = Quad;
		}
	}

	for (CLight* Light = Lights; Light < LastLight; Light++)
	{
		if (Light->Sphere)
		{
			if (Light->Sphere->Intersect(Origin, Ray, Data.Distance, Data.TestDistance, Data.TestPoint))
			{
				Data.Point = Data.TestPoint;
				Data.Distance = Data.TestDistance;
				Data.Light = Light;
			}
		}
		else
		{
			if (Light->Quad->Intersect(Origin, Ray, Data.Distance, Data.TestDistance, Data.TestPoint))
			{
				Data.Point = Data.TestPoint;
				Data.Distance = Data.TestDistance;
				Data.Light = Light;
			}
		}
	}

	if (Data.Light)
	{
		Data.Color = Data.Light->Sphere ? Data.Light->Sphere->Color : Data.Light->Quad->Color;
	}
	else if (Data.Quad)
	{
		Data.Color = Data.Quad->Color;

		if (Textures && Data.Quad->Texture)
		{
			float s = dot(Data.Quad->T, Data.Point) - Data.Quad->O.x;
			float t = dot(Data.Quad->B, Data.Point) - Data.Quad->O.y;

			Data.Color *= Data.Quad->Texture->GetColorBilinear(s, t);
		}

		IlluminatePoint(Data.Quad, Data.Point, Data.Quad->N, Data.Color);

		if (Data.Quad->Reflection > 0.0f)
		{
			vec3 ReflectedRay = reflect(Ray, Data.Quad->N);

			Data.Color = mix(Data.Color, RayTrace(Data.Point, ReflectedRay, Depth + 1, Data.Quad), Data.Quad->Reflection);
		}
	}
	else if (Data.Sphere)
	{
		Data.Color = Data.Sphere->Color;

		vec3 Normal = (Data.Point - Data.Sphere->Position) * Data.Sphere->ODRadius;

		if (Textures && Data.Sphere->Texture)
		{
			float s = atan2(Normal.x, Normal.z) * M_1_PI_2 + 0.5f;
			float t = asin(Normal.y < -1.0f ? -1.0f : Normal.y > 1.0f ? 1.0f : Normal.y) * (float)M_1_PI + 0.5f;

			Data.Color *= Data.Sphere->Texture->GetColorBilinear(s, t);
		}

		IlluminatePoint(Data.Sphere, Data.Point, Normal, Data.Color);

		if (Data.Sphere->Refraction > 0.0f)
		{
			vec3 RefractedRay = refract(Ray, Normal, Data.Sphere->ODEta);

			vec3 L = Data.Sphere->Position - Data.Point;
			float LdotRR = dot(L, RefractedRay);
			float D2 = length2(L) - LdotRR * LdotRR;
			float Distance = LdotRR + sqrt(Data.Sphere->Radius2 - D2);

			vec3 NewPoint = RefractedRay * Distance + Data.Point;

			vec3 NewNormal = (Data.Sphere->Position - NewPoint) * Data.Sphere->ODRadius;

			RefractedRay = refract(RefractedRay, NewNormal, Data.Sphere->Eta);

			Data.Color = mix(Data.Color, RayTrace(NewPoint, RefractedRay, Depth + 1, Data.Sphere), Data.Sphere->Refraction);
		}

		if (Data.Sphere->Reflection > 0.0f)
		{
			vec3 ReflectedRay = reflect(Ray, Normal);

			Data.Color = mix(Data.Color, RayTrace(Data.Point, ReflectedRay, Depth + 1, Data.Sphere), Data.Sphere->Reflection);
		}
	}

	return Data.Color;
}