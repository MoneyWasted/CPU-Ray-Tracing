#include "stdafx.h"

#include "CRayTracer.h"

#include "CQuad.h"
#include "CSphere.h"
#include "CLight.h"

CRayTracer::CRayTracer()
{
	ColorBuffer = NULL;
	HDRColorBuffer = NULL;

	Samples = 1;
	GISamples = 16;

	ODGISamples = 1.0f / (float)GISamples;
	AmbientOcclusionIntensity = 0.5f;
	ODGISamplesMAmbientOcclusionIntensity = ODGISamples * AmbientOcclusionIntensity;

	Quads = NULL;
	Spheres = NULL;
	Lights = NULL;

	LastQuad = NULL;
	LastSphere = NULL;
	LastLight = NULL;

	QuadsCount = 0;
	SpheresCount = 0;
	LightsCount = 0;

	Textures = true;
	SoftShadows = false;
	AmbientOcclusion = false;

	ULARGE_INTEGER time = {};
	time.QuadPart = GetTickCount64();
	srand(time.LowPart);
}

CRayTracer::~CRayTracer()
{
}

CQuad* CRayTracer::CreateQuads(int Count)
{
	QuadStorage.reset(Count > 0 ? new CQuad[Count] : NULL);
	Quads = QuadStorage.get();
	QuadsCount = Count;
	LastQuad = NULL;
	return Quads;
}

CSphere* CRayTracer::CreateSpheres(int Count)
{
	SphereStorage.reset(Count > 0 ? new CSphere[Count] : NULL);
	Spheres = SphereStorage.get();
	SpheresCount = Count;
	LastSphere = NULL;
	return Spheres;
}

CLight* CRayTracer::CreateLights(int Count)
{
	LightStorage.reset(Count > 0 ? new CLight[Count] : NULL);
	Lights = LightStorage.get();
	LightsCount = Count;
	LastLight = NULL;
	return Lights;
}

bool CRayTracer::Init()
{
	if (InitScene() == false)
	{
		return false;
	}

	LastQuad = Quads ? Quads + QuadsCount : NULL;
	LastSphere = Spheres ? Spheres + SpheresCount : NULL;
	LastLight = Lights ? Lights + LightsCount : NULL;

	return true;
}

void CRayTracer::RayTrace(int Line)
{
	if (ColorBuffer == NULL || HDRColorBuffer == NULL) return;

	Vector3* hdrcolorbuffer;
	BYTE* colorbuffer = LineWidth * Line * 3 + ColorBuffer;

	if (Samples == 1)
	{
		hdrcolorbuffer = Width * Line + HDRColorBuffer;

		for (int x = 0; x < Width; x++)
		{
			Vector3 Color = RayTrace(Camera.Position, normalize(Camera.RayMatrix * Vector3((float)x, (float)Line, 0.0f)));

			hdrcolorbuffer->r = Color.r;
			hdrcolorbuffer->g = Color.g;
			hdrcolorbuffer->b = Color.b;

			hdrcolorbuffer++;

			colorbuffer[2] = Color.r <= 0.0f ? 0 : Color.r >= 1.0 ? 255 : (BYTE)(Color.r * 255);
			colorbuffer[1] = Color.g <= 0.0f ? 0 : Color.g >= 1.0 ? 255 : (BYTE)(Color.g * 255);
			colorbuffer[0] = Color.b <= 0.0f ? 0 : Color.b >= 1.0 ? 255 : (BYTE)(Color.b * 255);

			colorbuffer += 3;
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

					hdrcolorbuffer->r = Color.r;
					hdrcolorbuffer->g = Color.g;
					hdrcolorbuffer->b = Color.b;

					hdrcolorbuffer++;

					SamplesSum.r += Color.r <= 0.0f ? 0.0f : Color.r >= 1.0 ? 1.0f : Color.r;
					SamplesSum.g += Color.g <= 0.0f ? 0.0f : Color.g >= 1.0 ? 1.0f : Color.g;
					SamplesSum.b += Color.b <= 0.0f ? 0.0f : Color.b >= 1.0 ? 1.0f : Color.b;
				}
			}

			SamplesSum.r *= ODSamples2;
			SamplesSum.g *= ODSamples2;
			SamplesSum.b *= ODSamples2;

			colorbuffer[2] = (BYTE)(SamplesSum.r * 255);
			colorbuffer[1] = (BYTE)(SamplesSum.g * 255);
			colorbuffer[0] = (BYTE)(SamplesSum.b * 255);

			colorbuffer += 3;
		}
	}
}

void CRayTracer::Resize(int Width, int Height)
{
	this->Width = Width;
	this->Height = Height;

	ColorBufferStorage.clear();
	HDRColorBufferStorage.clear();
	ColorBuffer = NULL;
	HDRColorBuffer = NULL;

	if (Width > 0 && Height > 0)
	{
		LineWidth = Width;

		int WidthMod4 = Width % 4;

		if (WidthMod4 > 0)
		{
			LineWidth += 4 - WidthMod4;
		}

		ColorBufferStorage.resize(LineWidth * Height * 3);
		ColorBuffer = ColorBufferStorage.data();

		memset(&ColorBufferInfo, 0, sizeof(BITMAPINFOHEADER));
		ColorBufferInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		ColorBufferInfo.bmiHeader.biPlanes = 1;
		ColorBufferInfo.bmiHeader.biBitCount = 24;
		ColorBufferInfo.bmiHeader.biCompression = BI_RGB;
		ColorBufferInfo.bmiHeader.biWidth = LineWidth;
		ColorBufferInfo.bmiHeader.biHeight = Height;

		WidthMSamples = Width * Samples;
		HeightMSamples = Height * Samples;
		WidthMHeightMSamples2 = WidthMSamples * HeightMSamples;
		ODSamples2 = 1.0f / (float)(Samples * Samples);

		HDRColorBufferStorage.resize(WidthMHeightMSamples2);
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
}

void CRayTracer::Destroy()
{
	DestroyTextures();

	QuadStorage.reset();
	SphereStorage.reset();
	LightStorage.reset();
	Quads = NULL;
	Spheres = NULL;
	Lights = NULL;
	QuadsCount = 0;
	SpheresCount = 0;
	LightsCount = 0;
	LastQuad = NULL;
	LastSphere = NULL;
	LastLight = NULL;

	ColorBufferStorage.clear();
	HDRColorBufferStorage.clear();
	ColorBuffer = NULL;
	HDRColorBuffer = NULL;
}

void CRayTracer::ClearColorBuffer()
{
	if (ColorBuffer != NULL)
	{
		memset(ColorBuffer, 0, LineWidth * Height * 3);
	}
}

int CRayTracer::GetSamples()
{
	return Samples * Samples;
}

void CRayTracer::MapHDRColors()
{
	if (ColorBuffer == NULL || HDRColorBuffer == NULL) return;

	float SumLum = 0.0f, LumWhite = 0.0f;
	int LumNotNull = 0;

	Vector3* Color = HDRColorBuffer;

	for (int i = 0; i < WidthMHeightMSamples2; i++)
	{
		float Luminance = (Color->r * 0.2125f + Color->g * 0.7154f + Color->b * 0.0721f);

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

	Vector3 ColorMMappingFactor;

	for (int i = 0; i < WidthMHeightMSamples2; i++)
	{
		float Luminance = (Color->r * 0.2125f + Color->g * 0.7154f + Color->b * 0.0721f);

		float LumRel = Luminance / AvgLum;
		float MappingFactor = LumRel * (1.0f + LumRel / LumWhite2) / (1.0f + LumRel);

		ColorMMappingFactor.r = Color->r * MappingFactor;
		ColorMMappingFactor.g = Color->g * MappingFactor;
		ColorMMappingFactor.b = Color->b * MappingFactor;

		Color->r = ColorMMappingFactor.r <= 0.0f ? 0.0f : ColorMMappingFactor.r >= 1.0f ? 1.0f : ColorMMappingFactor.r;
		Color->g = ColorMMappingFactor.g <= 0.0f ? 0.0f : ColorMMappingFactor.g >= 1.0f ? 1.0f : ColorMMappingFactor.g;
		Color->b = ColorMMappingFactor.b <= 0.0f ? 0.0f : ColorMMappingFactor.b >= 1.0f ? 1.0f : ColorMMappingFactor.b;

		Color++;
	}

	int LineWidthSWidthM3 = (LineWidth - Width) * 3;

	BYTE* colorbuffer = ColorBuffer;

	if (Samples == 1)
	{
		Color = HDRColorBuffer;

		for (int y = 0; y < Height; y++)
		{
			for (int x = 0; x < Width; x++)
			{
				colorbuffer[2] = (BYTE)(Color->r * 255);
				colorbuffer[1] = (BYTE)(Color->g * 255);
				colorbuffer[0] = (BYTE)(Color->b * 255);

				Color++;
				colorbuffer += 3;
			}

			colorbuffer += LineWidthSWidthM3;
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

				colorbuffer[2] = (BYTE)(ColorSum.r * 255);
				colorbuffer[1] = (BYTE)(ColorSum.g * 255);
				colorbuffer[0] = (BYTE)(ColorSum.b * 255);

				colorbuffer += 3;
			}

			colorbuffer += LineWidthSWidthM3;
		}
	}
}

bool CRayTracer::SetSamples(int Samples)
{
	if (this->Samples == Samples) return false;

	this->Samples = Samples;

	Resize(Width, Height);

	return true;
}

void CRayTracer::SwapBuffers(HDC hDC)
{
	if (ColorBuffer != NULL)
	{
		StretchDIBits(hDC, 0, 0, Width, Height, 0, 0, Width, Height, ColorBuffer, &ColorBufferInfo, DIB_RGB_COLORS, SRCCOPY);
	}
}