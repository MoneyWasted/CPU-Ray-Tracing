#include "stdafx.h"

#include "CGPURayTracer.h"
#include "CSphere.h"
#include "CQuad.h"
#include "CLight.h"
#include "CTexture.h"
#include "CCamera.h"

#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

CGPURayTracer::CGPURayTracer(ID3D11Device* device, ID3D11DeviceContext* context)
	: Device(device), Context(context), ComputeShader(nullptr),
	OutputUAV(nullptr), OutputTexture(nullptr), ReadbackBuffer(nullptr), SpheresBufferSRV(nullptr), QuadsBufferSRV(nullptr),
	LightsBufferSRV(nullptr), TextureInfoSRV(nullptr), TextureInfoBuffer(nullptr), TextureSampler(nullptr),
	SpheresBuffer(nullptr), QuadsBuffer(nullptr), LightsBuffer(nullptr),
	Width(0), Height(0), Samples(1), Textures(true), SoftShadows(false),
	AmbientOcclusion(false), AmbientOcclusionIntensity(0.5f), TextureCount(0)
{
	memset(TextureSRVs, 0, sizeof(TextureSRVs));
	memset(TextureMetadata, 0, sizeof(TextureMetadata));
}

CGPURayTracer::~CGPURayTracer()
{
	ReleaseResources();
}

void CGPURayTracer::ReleaseResources()
{
	if (ComputeShader) ComputeShader->Release();
	if (ConstantsBuffer) ConstantsBuffer->Release();
	if (OutputUAV) OutputUAV->Release();
	if (OutputTexture) OutputTexture->Release();
	if (ReadbackBuffer) ReadbackBuffer->Release();
	if (SpheresBufferSRV) SpheresBufferSRV->Release();
	if (QuadsBufferSRV) QuadsBufferSRV->Release();
	if (LightsBufferSRV) LightsBufferSRV->Release();
	if (TextureInfoSRV) TextureInfoSRV->Release();
	if (TextureInfoBuffer) TextureInfoBuffer->Release();
	if (TextureSampler) TextureSampler->Release();
	if (SpheresBuffer) SpheresBuffer->Release();
	if (QuadsBuffer) QuadsBuffer->Release();
	if (LightsBuffer) LightsBuffer->Release();

	for (int i = 0; i < MaxTextures; ++i)
	{
		if (TextureSRVs[i]) TextureSRVs[i]->Release();
	}

	TextureCount = 0;

	ComputeShader = nullptr;
	ConstantsBuffer = nullptr;
	OutputUAV = nullptr;
	OutputTexture = nullptr;
	ReadbackBuffer = nullptr;
	SpheresBufferSRV = nullptr;
	QuadsBufferSRV = nullptr;
	LightsBufferSRV = nullptr;
	TextureInfoSRV = nullptr;
	TextureInfoBuffer = nullptr;
	TextureSampler = nullptr;
	SpheresBuffer = nullptr;
	QuadsBuffer = nullptr;
	LightsBuffer = nullptr;
	memset(TextureSRVs, 0, sizeof(TextureSRVs));
}

bool CGPURayTracer::Init(int width, int height)
{
	Width = width;
	Height = height;
	OutputData.resize(static_cast<size_t>(width) * height);

	if (!CreateComputeShader() || !CreateBuffers())
	{
		ReleaseResources();
		return false;
	}

	return true;
}

bool CGPURayTracer::CreateComputeShader()
{
	// Precompiled compute shader is loaded from RayTracer.cso
	ID3DBlob* shaderBlob = nullptr;
	if (FAILED(D3DReadFileToBlob(L"RayTracer.cso", &shaderBlob)))
	{
		return false;
	}

	HRESULT hr = Device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &ComputeShader);
	shaderBlob->Release();

	return SUCCEEDED(hr);
}

bool CGPURayTracer::CreateBuffers()
{
	// Create constants buffer
	{
		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = sizeof(GPURayTraceConstants);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		if (FAILED(Device->CreateBuffer(&desc, nullptr, &ConstantsBuffer)))
		{
			return false;
		}
	}

	// Create output texture and UAV
	{
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = Width;
		desc.Height = Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		if (FAILED(Device->CreateTexture2D(&desc, nullptr, &OutputTexture)))
		{
			return false;
		}

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.Format = desc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		if (FAILED(Device->CreateUnorderedAccessView(OutputTexture, &uavDesc, &OutputUAV)))
		{
			return false;
		}
	}


	// Create texture sampler
	{
		D3D11_SAMPLER_DESC desc{};
		desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MaxLOD = FLT_MAX;

		if (FAILED(Device->CreateSamplerState(&desc, &TextureSampler)))
		{
			return false;
		}
	}


	return true;
}

GPUSphere CGPURayTracer::ConvertSphere(const CSphere& sphere, int textureIndex)
{
	GPUSphere gpu{};
	gpu.Position = sphere.Position;
	gpu.Radius = sphere.Radius;
	gpu.Color = sphere.Color;
	gpu.Reflection = sphere.Reflection;
	gpu.Refraction = sphere.Refraction;
	gpu.Eta = sphere.Eta;
	gpu.ODRadius = sphere.ODRadius;
	gpu.TextureIndex = textureIndex;
	return gpu;
}

GPUQuad CGPURayTracer::ConvertQuad(const CQuad& quad, int textureIndex)
{
	GPUQuad gpu{};
	gpu.a = quad.a;
	gpu.b = quad.b;
	gpu.c = quad.c;
	gpu.d = quad.d;
	gpu.Color = quad.Color;
	gpu.Reflection = quad.Reflection;
	gpu.Refraction = quad.Refraction;
	gpu.Eta = quad.Eta;
	gpu.ODEta = quad.ODEta;
	gpu.Normal = quad.N;
	gpu.Tangent = quad.T;
	gpu.Binormal = quad.B;
	gpu.m = quad.m;
	gpu.Offset = quad.O;
	gpu.D = quad.D;
	gpu.D1 = quad.D1;
	gpu.D2 = quad.D2;
	gpu.D3 = quad.D3;
	gpu.D4 = quad.D4;
	gpu.TextureIndex = textureIndex;
	return gpu;
}

GPULight CGPURayTracer::ConvertLight(const CLight& light, const std::vector<CSphere*>& spheres, const std::vector<CQuad*>& quads)
{
	GPULight gpu{};
	gpu.Ambient = light.Ambient;
	gpu.Diffuse = light.Diffuse;

	if (light.Sphere)
	{
		gpu.IsSphere = 1;
		gpu.Position = light.Sphere->Position;
		gpu.Radius = light.Sphere->Radius;
		gpu.Color = light.Sphere->Color;
		for (size_t i = 0; i < spheres.size(); ++i)
		{
			if (spheres[i] == light.Sphere)
			{
				gpu.ObjectIndex = static_cast<int>(i);
				break;
			}
		}
	}
	else
	{
		gpu.IsSphere = 0;
		gpu.Position = light.Quad->m;
		gpu.Radius = 0.5f;
		gpu.Color = light.Quad->Color;
		for (size_t i = 0; i < quads.size(); ++i)
		{
			if (quads[i] == light.Quad)
			{
				gpu.ObjectIndex = static_cast<int>(i);
				break;
			}
		}
	}

	return gpu;
}

bool CGPURayTracer::CreateTextureSRVs(const std::vector<CTexture*>& textures)
{
	for (UINT i = 0; i < MaxTextures; ++i)
	{
		if (TextureSRVs[i])
		{
			TextureSRVs[i]->Release();
			TextureSRVs[i] = nullptr;
		}
	}

	if (TextureInfoSRV)
	{
		TextureInfoSRV->Release();
		TextureInfoSRV = nullptr;
	}

	if (TextureInfoBuffer)
	{
		TextureInfoBuffer->Release();
		TextureInfoBuffer = nullptr;
	}

	TextureCount = 0;

	std::vector<TextureInfo> textureInfo;
	textureInfo.reserve(textures.size());

	for (size_t i = 0; i < textures.size() && i < MaxTextures; ++i)
	{
		const CTexture* texture = textures[i];
		if (texture == nullptr || !texture->HasData() || texture->GetWidth() <= 0 || texture->GetHeight() <= 0)
		{
			continue;
		}

		TextureInfo info{};
		info.Width = static_cast<unsigned int>(texture->GetWidth());
		info.Height = static_cast<unsigned int>(texture->GetHeight());
		textureInfo.push_back(info);

		std::vector<UINT> rgba(static_cast<size_t>(info.Width) * info.Height);
		const BYTE* src = texture->GetData();
		for (unsigned int y = 0; y < info.Height; ++y)
		{
			for (unsigned int x = 0; x < info.Width; ++x)
			{
				size_t srcIndex = (static_cast<size_t>(y) * info.Width + x) * 3;
				BYTE r = src[srcIndex + 0];
				BYTE g = src[srcIndex + 1];
				BYTE b = src[srcIndex + 2];
				rgba[static_cast<size_t>(y) * info.Width + x] = (0xFFu << 24) | (static_cast<UINT>(r) << 16) | (static_cast<UINT>(g) << 8) | static_cast<UINT>(b);
			}
		}

		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Width = info.Width;
		texDesc.Height = info.Height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_IMMUTABLE;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = rgba.data();
		data.SysMemPitch = info.Width * sizeof(UINT);

		ID3D11Texture2D* gpuTexture = nullptr;
		if (FAILED(Device->CreateTexture2D(&texDesc, &data, &gpuTexture)))
		{
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		if (FAILED(Device->CreateShaderResourceView(gpuTexture, &srvDesc, &TextureSRVs[TextureCount])))
		{
			gpuTexture->Release();
			return false;
		}

		gpuTexture->Release();
		++TextureCount;
	}

	if (!textureInfo.empty())
	{
		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = static_cast<UINT>(textureInfo.size() * sizeof(TextureInfo));
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = textureInfo.data();

		if (FAILED(Device->CreateBuffer(&desc, &data, &TextureInfoBuffer)))
		{
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = static_cast<UINT>(textureInfo.size());

		if (FAILED(Device->CreateShaderResourceView(TextureInfoBuffer, &srvDesc, &TextureInfoSRV)))
		{
			return false;
		}
	}

	return true;
}

bool CGPURayTracer::UpdateScene(
	const std::vector<CSphere*>& spheres,
	const std::vector<CQuad*>& quads,
	const std::vector<CLight*>& lights,
	const std::vector<CTexture*>& textures)
{
	GPUSpheres.clear();
	GPUQuads.clear();
	GPULights.clear();

	int textureIndex = 0;

	for (const auto& sphere : spheres)
	{
		int srvIdx = -1;
		if (sphere->Texture && textureIndex < MaxTextures)
		{
			srvIdx = textureIndex++;
		}
		GPUSpheres.push_back(ConvertSphere(*sphere, srvIdx));
	}

	for (const auto& quad : quads)
	{
		int srvIdx = -1;
		if (quad->Texture && textureIndex < MaxTextures)
		{
			srvIdx = textureIndex++;
		}
		GPUQuads.push_back(ConvertQuad(*quad, srvIdx));
	}

	for (const auto& light : lights)
	{
		GPULights.push_back(ConvertLight(*light, spheres, quads));
	}

	// Update sphere buffer
	{
		if (SpheresBuffer) SpheresBuffer->Release();
		SpheresBuffer = nullptr;
		if (SpheresBufferSRV) SpheresBufferSRV->Release();
		SpheresBufferSRV = nullptr;

		if (!GPUSpheres.empty())
		{
			D3D11_BUFFER_DESC desc{};
			desc.ByteWidth = static_cast<UINT>(GPUSpheres.size() * sizeof(GPUSphere));
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			D3D11_SUBRESOURCE_DATA data{};
			data.pSysMem = GPUSpheres.data();

			if (FAILED(Device->CreateBuffer(&desc, &data, &SpheresBuffer)))
			{
				return false;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = static_cast<UINT>(GPUSpheres.size());

			if (FAILED(Device->CreateShaderResourceView(SpheresBuffer, &srvDesc, &SpheresBufferSRV)))
			{
				return false;
			}
		}
	}

	// Update quad buffer
	{
		if (QuadsBuffer) QuadsBuffer->Release();
		QuadsBuffer = nullptr;
		if (QuadsBufferSRV) QuadsBufferSRV->Release();
		QuadsBufferSRV = nullptr;

		if (!GPUQuads.empty())
		{
			D3D11_BUFFER_DESC desc{};
			desc.ByteWidth = static_cast<UINT>(GPUQuads.size() * sizeof(GPUQuad));
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			D3D11_SUBRESOURCE_DATA data{};
			data.pSysMem = GPUQuads.data();

			if (FAILED(Device->CreateBuffer(&desc, &data, &QuadsBuffer)))
			{
				return false;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = static_cast<UINT>(GPUQuads.size());

			if (FAILED(Device->CreateShaderResourceView(QuadsBuffer, &srvDesc, &QuadsBufferSRV)))
			{
				return false;
			}
		}
	}

	// Update light buffer
	{
		if (LightsBuffer) LightsBuffer->Release();
		LightsBuffer = nullptr;
		if (LightsBufferSRV) LightsBufferSRV->Release();
		LightsBufferSRV = nullptr;

		if (!GPULights.empty())
		{
			D3D11_BUFFER_DESC desc{};
			desc.ByteWidth = static_cast<UINT>(GPULights.size() * sizeof(GPULight));
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			D3D11_SUBRESOURCE_DATA data{};
			data.pSysMem = GPULights.data();

			if (FAILED(Device->CreateBuffer(&desc, &data, &LightsBuffer)))
			{
				return false;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = static_cast<UINT>(GPULights.size());

			if (FAILED(Device->CreateShaderResourceView(LightsBuffer, &srvDesc, &LightsBufferSRV)))
			{
				return false;
			}
		}
	}

	if (!CreateTextureSRVs(textures))
	{
		return false;
	}

	return true;
}

bool CGPURayTracer::Resize(int width, int height)
{
	Width = width;
	Height = height;
	OutputData.resize(static_cast<size_t>(width) * height);

	if (OutputUAV) OutputUAV->Release();
	if (OutputTexture) OutputTexture->Release();
	if (ReadbackBuffer) ReadbackBuffer->Release();

	OutputUAV = nullptr;
	OutputTexture = nullptr;
	ReadbackBuffer = nullptr;

	return CreateBuffers();
}

bool CGPURayTracer::Trace(const CCamera& camera, int samples, bool textures, bool softShadows, bool ambientOcclusion, float aoIntensity)
{
	if (!ComputeShader || !ConstantsBuffer || !OutputUAV || !OutputTexture)
	{
		return false;
	}

	Samples = samples;
	Textures = textures;
	SoftShadows = softShadows;
	AmbientOcclusion = ambientOcclusion;
	AmbientOcclusionIntensity = aoIntensity;

	// Update constants
	{
		D3D11_MAPPED_SUBRESOURCE mapped{};
		if (SUCCEEDED(Context->Map(ConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		{
			GPURayTraceConstants* consts = static_cast<GPURayTraceConstants*>(mapped.pData);
			consts->Width = Width;
			consts->Height = Height;
			consts->Samples = Samples;
			consts->GISamples = 16;
			consts->FrameIndex = 0;
			consts->MaxDepth = 8;
			consts->AmbientOcclusionIntensity = AmbientOcclusionIntensity;
			consts->Textures = Textures ? 1 : 0;
			consts->SoftShadows = SoftShadows ? 1 : 0;
			consts->AmbientOcclusion = AmbientOcclusion ? 1 : 0;
			consts->NumSpheres = static_cast<unsigned int>(GPUSpheres.size());
			consts->NumQuads = static_cast<unsigned int>(GPUQuads.size());
			consts->NumLights = static_cast<unsigned int>(GPULights.size());
			consts->NumTextures = TextureCount;
			consts->CameraPosition = camera.Position;
			memcpy(&consts->CameraRayMatrix, &camera.RayMatrix, sizeof(camera.RayMatrix));

			Context->Unmap(ConstantsBuffer, 0);
		}
	}

	// Set compute shader and buffers
	Context->CSSetShader(ComputeShader, nullptr, 0);
	Context->CSSetConstantBuffers(0, 1, &ConstantsBuffer);
	Context->CSSetUnorderedAccessViews(0, 1, &OutputUAV, nullptr);

	ID3D11ShaderResourceView* srvs[20] = {};
	for (UINT i = 0; i < TextureCount; ++i)
	{
		srvs[3 + i] = TextureSRVs[i];
	}
	srvs[0] = SpheresBufferSRV;
	srvs[1] = QuadsBufferSRV;
	srvs[2] = LightsBufferSRV;
	srvs[19] = TextureInfoSRV;
	Context->CSSetShaderResources(0, 20, srvs);
	Context->CSSetSamplers(0, 1, &TextureSampler);

	// Dispatch compute shader (8x8 threads = 64 per group)
	UINT groupsX = (Width + 7) / 8;
	UINT groupsY = (Height + 7) / 8;
	Context->Dispatch(groupsX, groupsY, 1);

	// Clear bindings
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	Context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	ID3D11ShaderResourceView* nullSRVs[20] = {};
	Context->CSSetShaderResources(0, 20, nullSRVs);

	return true;
}

bool CGPURayTracer::Present(ID3D11Texture2D* backBufferTexture)
{
	if (!OutputTexture || !backBufferTexture)
	{
		return false;
	}

	Context->CopyResource(backBufferTexture, OutputTexture);
	return true;
}
