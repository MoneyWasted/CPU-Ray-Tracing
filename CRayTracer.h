#pragma once

#include <memory>
#include <vector>
#include <windows.h>

#include "glmath.h"
#include "CCamera.h"

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
	std::vector<Vector3> HDRColorBufferStorage;
	Vector3* HDRColorBuffer;
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
	bool Shadow(void* Object, Vector3& Point, Vector3& LightDirection, float LightDistance);
	Vector3 LightIntensity(void* Object, Vector3& Point, Vector3& Normal, Vector3& LightPosition, CLight* Light, float AO);
	float AmbientOcclusionFactor(void* Object, Vector3& Point, Vector3& Normal);
	void IlluminatePoint(void* Object, Vector3& Point, Vector3& Normal, Vector3& Color);
	Vector3 RayTrace(Vector3& Origin, const Vector3& Ray, UINT Depth = 0, void* Object = NULL);
};