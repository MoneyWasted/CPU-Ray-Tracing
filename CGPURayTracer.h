#pragma once

#include <d3d11.h>
#include <vector>
#include "GPUStructures.h"

class CTexture;
class CSphere;
class CQuad;
class CLight;
class CCamera;

class CGPURayTracer
{
private:
	// D3D11 resources
	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
	ID3D11ComputeShader* ComputeShader;
	ID3D11Buffer* ConstantsBuffer;
	ID3D11UnorderedAccessView* OutputUAV;
	ID3D11Texture2D* OutputTexture;
	ID3D11Buffer* ReadbackBuffer;
	ID3D11ShaderResourceView* SpheresBufferSRV;
	ID3D11ShaderResourceView* QuadsBufferSRV;
	ID3D11ShaderResourceView* LightsBufferSRV;
	ID3D11ShaderResourceView* TextureInfoSRV;
	ID3D11Buffer* TextureInfoBuffer;
	ID3D11SamplerState* TextureSampler;

	// GPU texture array (max 16 textures)
	static constexpr UINT MaxTextures = 16;
	ID3D11ShaderResourceView* TextureSRVs[MaxTextures];
	TextureInfo TextureMetadata[MaxTextures];
	UINT TextureCount;

	// Scene data buffers
	std::vector<GPUSphere> GPUSpheres;
	std::vector<GPUQuad> GPUQuads;
	std::vector<GPULight> GPULights;
	ID3D11Buffer* SpheresBuffer;
	ID3D11Buffer* QuadsBuffer;
	ID3D11Buffer* LightsBuffer;

	// Output
	std::vector<UINT> OutputData;
	int Width, Height;
	int Samples;
	bool Textures, SoftShadows, AmbientOcclusion;
	float AmbientOcclusionIntensity;

	bool CreateComputeShader();
	bool CreateBuffers();
	bool CreateTextureSRVs(const std::vector<CTexture*>& textures);
	void ReleaseResources();
	GPUSphere ConvertSphere(const CSphere& sphere, int textureIndex);
	GPUQuad ConvertQuad(const CQuad& quad, int textureIndex);
	GPULight ConvertLight(const CLight& light, const std::vector<CSphere*>& spheres, const std::vector<CQuad*>& quads);

public:
	CGPURayTracer(ID3D11Device* device, ID3D11DeviceContext* context);
	~CGPURayTracer();

	bool Init(int width, int height);
	bool UpdateScene(
		const std::vector<CSphere*>& spheres,
		const std::vector<CQuad*>& quads,
		const std::vector<CLight*>& lights,
		const std::vector<CTexture*>& textures);
	bool Resize(int width, int height);
	bool Trace(const CCamera& camera, int samples, bool textures, bool softShadows, bool ambientOcclusion, float aoIntensity);
	bool Present(ID3D11Texture2D* backBufferTexture);
	const UINT* GetOutputData() const { return OutputData.data(); }
	bool HasOutput() const { return !OutputData.empty(); }
};
