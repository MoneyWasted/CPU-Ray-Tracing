#pragma once

#include <memory>
#include <vector>
#include <windows.h>

#include "glmath.h"
#include "Camera.h"

class CQuad;
class CSphere;
class CLight;

extern CCamera Camera;

class CRayTracer
{
private:
	std::vector<BYTE> ColorBufferStorage;
	BYTE* ColorBuffer;
	BITMAPINFO ColorBufferInfo;
	std::vector<vec3> HDRColorBufferStorage;
	vec3* HDRColorBuffer;
	int Width, LineWidth, Height, Samples, GISamples, WidthMSamples, HeightMSamples, WidthMHeightMSamples2;
	float ODSamples2, ODGISamples, AmbientOcclusionIntensity, ODGISamplesMAmbientOcclusionIntensity;

protected:
	std::unique_ptr<CQuad[]> QuadStorage;
	std::unique_ptr<CSphere[]> SphereStorage;
	std::unique_ptr<CLight[]> LightStorage;
	CQuad* Quads, * LastQuad;
	CSphere* Spheres, * LastSphere;
	CLight* Lights, * LastLight;
	int QuadsCount, SpheresCount, LightsCount;

public:
	bool Textures, SoftShadows, AmbientOcclusion;

public:
	CRayTracer();
	~CRayTracer();

	bool Init();
	void RayTrace(int Line);
	void Resize(int Width, int Height);
	void Destroy();

	void ClearColorBuffer();
	int GetSamples();
	void MapHDRColors();
	bool SetSamples(int Samples);
	void SwapBuffers(HDC hDC);

protected:
	virtual bool InitScene() = 0;
	virtual void DestroyTextures() = 0;
	CQuad* CreateQuads(int Count);
	CSphere* CreateSpheres(int Count);
	CLight* CreateLights(int Count);

private:
	bool Shadow(void* Object, vec3& Point, vec3& LightDirection, float LightDistance);
	vec3 LightIntensity(void* Object, vec3& Point, vec3& Normal, vec3& LightPosition, CLight* Light, float AO);
	float AmbientOcclusionFactor(void* Object, vec3& Point, vec3& Normal);
	void IlluminatePoint(void* Object, vec3& Point, vec3& Normal, vec3& Color);
	vec3 RayTrace(vec3& Origin, const vec3& Ray, UINT Depth = 0, void* Object = NULL);
};