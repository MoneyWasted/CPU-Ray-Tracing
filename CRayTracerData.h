#pragma once

#include "glmath.h"

class CQuad;
class CSphere;
class CLight;

struct CRayTracerData
{
public:
	float Distance, TestDistance;
	Vector3 Color, Point, TestPoint;
	CQuad* Quad;
	CSphere* Sphere;
	CLight* Light;
	CRayTracerData();
};