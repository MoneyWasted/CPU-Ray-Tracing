#pragma once

class CQuad;
class CSphere;

class CLight
{
public:
	float Ambient, Diffuse, ConstantAttenuation, LinearAttenuation, QuadraticAttenuation;
	CSphere* Sphere;
	CQuad* Quad;
	CLight();
	~CLight();
};