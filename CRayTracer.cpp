#include "stdafx.h"

#include "CRayTracer.h"

#include "CQuad.h"
#include "CSphere.h"
#include "CLight.h"
#include "CTexture.h"
#include "CGPURayTracer.h"
#include "CWindow.h"

extern CWindow Wnd;

namespace
{
	float Clamp01(float value)
	{
		if (value <= 0.0f) return 0.0f;
		if (value >= 1.0f) return 1.0f;
		return value;
	}

	BYTE ToByte(float value)
	{
		return (BYTE)(Clamp01(value) * 255.0f);
	}
}

CRayTracer::CRayTracer() :
	ColorBuffer(nullptr),
	HDRColorBuffer(nullptr),
	Width(0), Height(0),
	Samples(1),
	GISamples(16),
	WidthMSamples(0), HeightMSamples(0), WidthMHeightMSamples2(0),
	ODSamples2(1.0f),
	ODGISamples(1.0f / 16.0f),
	AmbientOcclusionIntensity(0.5f),
	ODGISamplesMAmbientOcclusionIntensity(1.0f / 16.0f * 0.5f),
	Quads(nullptr), LastQuad(nullptr),
	Spheres(nullptr), LastSphere(nullptr),
	Lights(nullptr), LastLight(nullptr),
	QuadsCount(0), SpheresCount(0), LightsCount(0),
	Textures(true), SoftShadows(false), AmbientOcclusion(false),
	GPUTracer(nullptr), UseGPUTracer(false), SceneDirty(true), SceneUploaded(false)
{
	srand((unsigned int)GetTickCount64());
}

CRayTracer::~CRayTracer()
{
}

CQuad* CRayTracer::CreateQuads(int Count)
{
	QuadStorage.reset(Count > 0 ? new CQuad[static_cast<size_t>(Count)] : nullptr);
	Quads = QuadStorage.get();
	QuadsCount = Count;
	LastQuad = nullptr;
	return Quads;
}

CSphere* CRayTracer::CreateSpheres(int Count)
{
	SphereStorage.reset(Count > 0 ? new CSphere[static_cast<size_t>(Count)] : nullptr);
	Spheres = SphereStorage.get();
	SpheresCount = Count;
	LastSphere = nullptr;
	return Spheres;
}

CLight* CRayTracer::CreateLights(int Count)
{
	LightStorage.reset(Count > 0 ? new CLight[static_cast<size_t>(Count)] : nullptr);
	Lights = LightStorage.get();
	LightsCount = Count;
	LastLight = nullptr;
	return Lights;
}

bool CRayTracer::Init()
{
	if (!InitScene())
	{
		return false;
	}

	SceneDirty = true;
	SceneUploaded = false;

	LastQuad = Quads ? Quads + QuadsCount : nullptr;
	LastSphere = Spheres ? Spheres + SpheresCount : nullptr;
	LastLight = Lights ? Lights + LightsCount : nullptr;

	return true;
}

void CRayTracer::RayTrace(int Line)
{
	if (ColorBuffer == nullptr || HDRColorBuffer == nullptr) return;

	Vector3* hdrcolorbuffer;
	BYTE* colorbuffer = static_cast<size_t>(Width) * static_cast<size_t>(Line) * 4 + ColorBuffer;

	// Legacy CPU rendering path remains for fallback only.
	ID3D11Device* device = Wnd.GetD3DDevice();
	ID3D11DeviceContext* context = Wnd.GetD3DDeviceContext();
	UNREFERENCED_PARAMETER(device);
	UNREFERENCED_PARAMETER(context);

	if (Samples == 1)
	{
		hdrcolorbuffer = Width * Line + HDRColorBuffer;

		for (int x = 0; x < Width; x++)
		{
			Vector3 Color = RayTrace(Camera.Position, normalize(Camera.RayMatrix * Vector3((float)x, (float)Line, 0.0f)));

			*hdrcolorbuffer++ = Color;

			colorbuffer[0] = ToByte(Color.b);
			colorbuffer[1] = ToByte(Color.g);
			colorbuffer[2] = ToByte(Color.r);
			colorbuffer[3] = 255;

			colorbuffer += 4;
		}
	}
	else
	{
		int Y = Line * Samples;

		for (int X = 0; X < WidthMSamples; X += Samples)
		{
			Vector3 SamplesSum;

			for (int yy = 0; yy < Samples; yy++)
			{
				int Yyy = Y + yy;

				hdrcolorbuffer = WidthMSamples * Yyy + X + HDRColorBuffer;

				for (int xx = 0; xx < Samples; xx++)
				{
					Vector3 Color = RayTrace(Camera.Position, normalize(Camera.RayMatrix * Vector3((float)(X + xx), (float)Yyy, 0.0f)));

					*hdrcolorbuffer++ = Color;

					SamplesSum.r += Clamp01(Color.r);
					SamplesSum.g += Clamp01(Color.g);
					SamplesSum.b += Clamp01(Color.b);
				}
			}

			SamplesSum.r *= ODSamples2;
			SamplesSum.g *= ODSamples2;
			SamplesSum.b *= ODSamples2;

			colorbuffer[0] = ToByte(SamplesSum.b);
			colorbuffer[1] = ToByte(SamplesSum.g);
			colorbuffer[2] = ToByte(SamplesSum.r);
			colorbuffer[3] = 255;

			colorbuffer += 4;
		}
	}
}

void CRayTracer::Resize(int newWidth, int newHeight)
{
	Width = newWidth;
	Height = newHeight;
	SceneDirty = true;

	ColorBufferStorage.clear();
	HDRColorBufferStorage.clear();
	ColorBuffer = nullptr;
	HDRColorBuffer = nullptr;

	if (Width > 0 && Height > 0)
	{
		ColorBufferStorage.resize(static_cast<size_t>(Width) * static_cast<size_t>(Height) * 4);
		ColorBuffer = ColorBufferStorage.data();

		WidthMSamples = Width * Samples;
		HeightMSamples = Height * Samples;
		WidthMHeightMSamples2 = WidthMSamples * HeightMSamples;
		ODSamples2 = 1.0f / (float)(Samples * Samples);

		HDRColorBufferStorage.resize(static_cast<size_t>(WidthMHeightMSamples2));
		HDRColorBuffer = HDRColorBufferStorage.data();

		Camera.VPin[0] = 1.0f / (float)(WidthMSamples - 1);
		Camera.VPin[5] = 1.0f / (float)(HeightMSamples - 1);

		float tany = tanf(45.0f / 360.0f * (float)M_PI), aspect = (float)Width / (float)Height;

		Camera.Pin[0] = tany * aspect;
		Camera.Pin[5] = tany;
		Camera.Pin[10] = 0.0f;
		Camera.Pin[14] = -1.0f;

		Camera.CalculateRayMatrix();
	}

	if (GPUTracer)
	{
		GPUTracer->Resize(Width, Height);
	}
}

void CRayTracer::Destroy()
{
	DestroyTextures();

	GPUTracer.reset();
	UseGPUTracer = false;

	QuadStorage.reset();
	SphereStorage.reset();
	LightStorage.reset();
	Quads = nullptr;
	Spheres = nullptr;
	Lights = nullptr;
	QuadsCount = 0;
	SpheresCount = 0;
	LightsCount = 0;
	LastQuad = nullptr;
	LastSphere = nullptr;
	LastLight = nullptr;

	ColorBufferStorage.clear();
	HDRColorBufferStorage.clear();
	ColorBuffer = nullptr;
	HDRColorBuffer = nullptr;
}

void CRayTracer::ClearColorBuffer()
{
	if (ColorBuffer != nullptr)
	{
		memset(ColorBuffer, 0, static_cast<size_t>(Width) * static_cast<size_t>(Height) * 4);
	}
}

int CRayTracer::GetSamples()
{
	return Samples * Samples;
}

void CRayTracer::CollectSceneTextures(std::vector<CTexture*>& textures) const
{
	UNREFERENCED_PARAMETER(textures);
}

bool CRayTracer::UseGPUBackend(ID3D11Device* device, ID3D11DeviceContext* context, int width, int height)
{
	if (device == nullptr || context == nullptr)
	{
		return false;
	}

	GPUTracer.reset(new CGPURayTracer(device, context));
	if (!GPUTracer->Init(width, height))
	{
		GPUTracer.reset();
		return false;
	}

	UseGPUTracer = true;
	return true;
}

void CRayTracer::DisableGPUBackend()
{
	GPUTracer.reset();
	UseGPUTracer = false;
}

bool CRayTracer::RenderGPU(const CCamera& camera)
{
	if (Width <= 0 || Height <= 0)
	{
		return false;
	}

	if (!GPUTracer)
	{
		ID3D11Device* device = Wnd.GetD3DDevice();
		ID3D11DeviceContext* context = Wnd.GetD3DDeviceContext();
		if (device == nullptr || context == nullptr)
		{
			return false;
		}

		if (!UseGPUBackend(device, context, Width, Height))
		{
			return false;
		}
	}

	std::vector<CTexture*> textures;
	if (SceneDirty || !SceneUploaded)
	{
		CollectSceneTextures(textures);
	}

	std::vector<CSphere*> spheres;
	spheres.reserve(static_cast<size_t>(SpheresCount));
	for (int i = 0; i < SpheresCount; ++i)
	{
		spheres.push_back(&Spheres[i]);
	}

	std::vector<CQuad*> quads;
	quads.reserve(static_cast<size_t>(QuadsCount));
	for (int i = 0; i < QuadsCount; ++i)
	{
		quads.push_back(&Quads[i]);
	}

	std::vector<CLight*> lights;
	lights.reserve(static_cast<size_t>(LightsCount));
	for (int i = 0; i < LightsCount; ++i)
	{
		lights.push_back(&Lights[i]);
	}

	if (SceneDirty || !SceneUploaded)
	{
		if (!GPUTracer->UpdateScene(spheres, quads, lights, textures))
		{
			return false;
		}

		SceneDirty = false;
		SceneUploaded = true;
	}

	return GPUTracer->Trace(camera, Samples, Textures, SoftShadows, AmbientOcclusion, AmbientOcclusionIntensity);
}

bool CRayTracer::PresentGPU(ID3D11Texture2D* backBufferTexture)
{
	return GPUTracer && GPUTracer->Present(backBufferTexture);
}

void CRayTracer::MapHDRColors()
{
	if (ColorBuffer == nullptr || HDRColorBuffer == nullptr) return;

	static constexpr float LumR = 0.2125f, LumG = 0.7154f, LumB = 0.0721f;

	float SumLum = 0.0f, LumWhite = 0.0f;
	int LumNotNull = 0;

	Vector3* Color = HDRColorBuffer;

	for (int i = 0; i < WidthMHeightMSamples2; i++)
	{
		float Luminance = Color->r * LumR + Color->g * LumG + Color->b * LumB;

		if (Luminance > 0.0f)
		{
			SumLum += Luminance;
			LumNotNull++;
			LumWhite = LumWhite > Luminance ? LumWhite : Luminance;
		}

		Color++;
	}

	if (LumNotNull == 0)
	{
		ClearColorBuffer();
		return;
	}

	float AvgLum = SumLum / (float)LumNotNull;

	LumWhite /= AvgLum;

	float LumWhite2 = LumWhite * LumWhite;

	Color = HDRColorBuffer;

	for (int i = 0; i < WidthMHeightMSamples2; i++)
	{
		float Luminance = Color->r * LumR + Color->g * LumG + Color->b * LumB;

		float LumRel = Luminance / AvgLum;
		float MappingFactor = LumRel * (1.0f + LumRel / LumWhite2) / (1.0f + LumRel);

		Color->r = Clamp01(Color->r * MappingFactor);
		Color->g = Clamp01(Color->g * MappingFactor);
		Color->b = Clamp01(Color->b * MappingFactor);

		Color++;
	}

	BYTE* colorbuffer = ColorBuffer;

	if (Samples == 1)
	{
		Color = HDRColorBuffer;

		for (int y = 0; y < Height; y++)
		{
			for (int x = 0; x < Width; x++)
			{
				colorbuffer[0] = ToByte(Color->b);
				colorbuffer[1] = ToByte(Color->g);
				colorbuffer[2] = ToByte(Color->r);
				colorbuffer[3] = 255;

				Color++;
				colorbuffer += 4;
			}
		}
	}
	else
	{
		for (int y = 0, Y = 0; y < Height; y++, Y += Samples)
		{
			for (int X = 0; X < WidthMSamples; X += Samples)
			{
				Vector3 ColorSum;

				for (int yy = 0; yy < Samples; yy++)
				{
					Color = WidthMSamples * (Y + yy) + X + HDRColorBuffer;

					for (int xx = 0; xx < Samples; xx++)
					{
						ColorSum.r += Color->r;
						ColorSum.g += Color->g;
						ColorSum.b += Color->b;

						Color++;
					}
				}

				ColorSum.r *= ODSamples2;
				ColorSum.g *= ODSamples2;
				ColorSum.b *= ODSamples2;

				colorbuffer[0] = ToByte(ColorSum.b);
				colorbuffer[1] = ToByte(ColorSum.g);
				colorbuffer[2] = ToByte(ColorSum.r);
				colorbuffer[3] = 255;

				colorbuffer += 4;
			}
		}
	}
}

bool CRayTracer::SetSamples(int newSamples)
{
	if (Samples == newSamples) return false;

	Samples = newSamples;

	Resize(Width, Height);

	return true;
}

