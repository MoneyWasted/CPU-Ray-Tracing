#include "stdafx.h"

#include "CLight.h"
#include "CSphere.h"
#include "CQuad.h"

CLight::CLight()
{
	Ambient = 1.0f;
	Diffuse = 1.0f;

	ConstantAttenuation = 1.0f;
	LinearAttenuation = 0.0f;
	QuadraticAttenuation = 0.0f;

	Sphere = nullptr;
	Quad = nullptr;
}

CLight::~CLight()
{
	if (Sphere)
	{
		delete Sphere;
	}
	else
	{
		delete Quad;
	}
}