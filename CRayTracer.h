#pragma once

#include <memory>
#include <vector>
#include <windows.h>
#include <d3d11.h>

#include "glmath.h"
#include "CCamera.h"

class CQuad;
class CSphere;
class CLight;
class CTexture;
class CGPURayTracer;

extern CCamera Camera;

class CRayTracer
{
private:
	std::vector<BYTE> ColorBufferStorage;
	BYTE* ColorBuffer;
	std::vector<Vector3> HDRColorBufferStorage;
	Vector3* HDRColorBuffer;
	int Width, Height, Samples, GISamples, WidthMSamples, HeightMSamples, WidthMHeightMSamples2;
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

	CRayTracer();
	virtual ~CRayTracer();

	CRayTracer(const CRayTracer&) = delete;
	CRayTracer& operator=(const CRayTracer&) = delete;
	CRayTracer(CRayTracer&&) = delete;
	CRayTracer& operator=(CRayTracer&&) = delete;

	bool Init();
	void RayTrace(int Line);
	void Resize(int Width, int Height);
	void Destroy();

	void ClearColorBuffer();
	int GetSamples();
	void MapHDRColors();
	bool SetSamples(int Samples);

	bool UseGPUBackend(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height);
	void DisableGPUBackend();
	bool RenderGPU(const CCamera& camera);
	bool PresentGPU(ID3D11Texture2D* backBufferTexture);
	bool HasGPUBackend() const { return GPUTracer != nullptr; }
	void MarkSceneDirty() { SceneDirty = true; }

protected:
	virtual bool InitScene() = 0;
	virtual void DestroyTextures() = 0;
	CQuad* CreateQuads(int Count);
	CSphere* CreateSpheres(int Count);
	CLight* CreateLights(int Count);
	virtual void CollectSceneTextures(std::vector<CTexture*>& textures) const;

private:
	bool Shadow(void* Object, const Vector3& Point, const Vector3& LightDirection, float LightDistance);
	Vector3 LightIntensity(void* Object, const Vector3& Point, const Vector3& Normal, const Vector3& LightPosition, CLight* Light, float AO);
	float AmbientOcclusionFactor(void* Object, const Vector3& Point, const Vector3& Normal);
	void IlluminatePoint(void* Object, const Vector3& Point, const Vector3& Normal, Vector3& Color);
	Vector3 RayTrace(const Vector3& Origin, const Vector3& Ray, UINT Depth = 0, void* Object = nullptr);

	std::unique_ptr<CGPURayTracer> GPUTracer;
	bool UseGPUTracer;
	bool SceneDirty;
	bool SceneUploaded;
};