#include "stdafx.h"

#include "CRayTracerData.h"

CRayTracerData::CRayTracerData()
{
	TestDistance = 0.0f;
	Distance = 1048576.0f;

	Quad = NULL;
	Light = NULL;
	Sphere = NULL;
}