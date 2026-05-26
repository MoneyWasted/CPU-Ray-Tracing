#pragma once

#include "glmath.h"

class CQuad;
class CSphere;
class CLight;

struct RTData
{
public:
	float Distance, TestDistance;
	vec3 Color, Point, TestPoint;
	CQuad* Quad;
	CSphere* Sphere;
	CLight* Light;
	RTData();
};