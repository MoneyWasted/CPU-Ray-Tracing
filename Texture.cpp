#include "stdafx.h"
#include "Texture.h"

#pragma comment(lib, "Windowscodecs.lib")

extern CString ModuleDirectory, ErrorLog;
float OD255 = 1.0f / 255;

CTexture::CTexture()
{
	Data = NULL;
	Width = Height = 0;
}

CTexture::~CTexture()
{
}

bool CTexture::CreateTexture2D(const char* Texture2DFileName)
{
	CString FileName = ModuleDirectory + Texture2DFileName;
	CString ErrorText = "Error loading file " + FileName + "! -> ";

	int FileNameWLength = MultiByteToWideChar(CP_ACP, 0, FileName, -1, NULL, 0);

	if (FileNameWLength <= 0)
	{
		ErrorLog.Append(ErrorText + "MultiByteToWideChar failed" + "\r\n");
		return false;
	}

	wchar_t* FileNameW = new wchar_t[FileNameWLength];

	if (MultiByteToWideChar(CP_ACP, 0, FileName, -1, FileNameW, FileNameWLength) == 0)
	{
		delete[] FileNameW;
		ErrorLog.Append(ErrorText + "MultiByteToWideChar failed" + "\r\n");
		return false;
	}

	IWICImagingFactory* Factory = NULL;
	IWICBitmapDecoder* Decoder = NULL;
	IWICBitmapFrameDecode* Frame = NULL;
	IWICFormatConverter* Converter = NULL;

	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&Factory));

	if (FAILED(hr))
	{
		delete[] FileNameW;
		ErrorLog.Append(ErrorText + "CoCreateInstance failed" + "\r\n");
		return false;
	}

	hr = Factory->CreateDecoderFromFilename(FileNameW, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &Decoder);

	delete[] FileNameW;

	if (FAILED(hr))
	{
		SafeRelease(Factory);
		ErrorLog.Append(ErrorText + "CreateDecoderFromFilename failed" + "\r\n");
		return false;
	}

	hr = Decoder->GetFrame(0, &Frame);

	if (FAILED(hr))
	{
		SafeRelease(Decoder);
		SafeRelease(Factory);
		ErrorLog.Append(ErrorText + "GetFrame failed" + "\r\n");
		return false;
	}

	hr = Factory->CreateFormatConverter(&Converter);

	if (FAILED(hr))
	{
		SafeRelease(Frame);
		SafeRelease(Decoder);
		SafeRelease(Factory);
		ErrorLog.Append(ErrorText + "CreateFormatConverter failed" + "\r\n");
		return false;
	}

	hr = Converter->Initialize(Frame, GUID_WICPixelFormat24bppBGR, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);

	if (FAILED(hr))
	{
		SafeRelease(Converter);
		SafeRelease(Frame);
		SafeRelease(Decoder);
		SafeRelease(Factory);
		ErrorLog.Append(ErrorText + "Format conversion failed" + "\r\n");
		return false;
	}

	UINT Width = 0, Height = 0;
	hr = Converter->GetSize(&Width, &Height);

	if (FAILED(hr) || Width == 0 || Height == 0)
	{
		SafeRelease(Converter);
		SafeRelease(Frame);
		SafeRelease(Decoder);
		SafeRelease(Factory);
		ErrorLog.Append(ErrorText + "Width or Height is 0" + "\r\n");
		return false;
	}

	Destroy();

	Data = new BYTE[Width * Height * 3];

	this->Width = Width;
	this->Height = Height;

	UINT Pitch = Width * 3;
	UINT BufferSize = Pitch * Height;
	BYTE* Bits = new BYTE[BufferSize];

	hr = Converter->CopyPixels(NULL, Pitch, BufferSize, Bits);

	if (FAILED(hr))
	{
		delete[] Bits;
		Destroy();
		SafeRelease(Converter);
		SafeRelease(Frame);
		SafeRelease(Decoder);
		SafeRelease(Factory);
		ErrorLog.Append(ErrorText + "CopyPixels failed" + "\r\n");
		return false;
	}

	BYTE* data = Data, * line = Bits;

	for (UINT y = 0; y < Height; y++)
	{
		BYTE* pixel = line;

		for (UINT x = 0; x < Width; x++)
		{
			data[0] = pixel[2];
			data[1] = pixel[1];
			data[2] = pixel[0];

			pixel += 3;
			data += 3;
		}

		line += Pitch;
	}

	delete[] Bits;

	SafeRelease(Converter);
	SafeRelease(Frame);
	SafeRelease(Decoder);
	SafeRelease(Factory);

	return true;
}

vec3 CTexture::GetColorNearest(float s, float t)
{
	vec3 Color = vec3(1.0f);

	if (Data != NULL)
	{
		s -= (int)s;
		t -= (int)t;

		if (s < 0.0f) s += 1.0f;
		if (t < 0.0f) t += 1.0f;

		int x = (int)(s * Width), y = (int)(t * Height);

		BYTE* data = (Width * y + x) * 3 + Data;

		Color.r = OD255 * data[0];
		Color.g = OD255 * data[1];
		Color.b = OD255 * data[2];
	}

	return Color;
}

vec3 CTexture::GetColorBilinear(float s, float t)
{
	vec3 Color = vec3(1.0f);

	if (Data != NULL)
	{
		s -= (int)s;
		t -= (int)t;

		if (s < 0.0f) s += 1.0f;
		if (t < 0.0f) t += 1.0f;

		float dx = s * Width - 0.5f, dy = t * Height - 0.5f;

		if (dx < 0.0f) dx += Width;
		if (dy < 0.0f) dy += Height;

		int x0 = (int)dx, y0 = (int)dy, x1 = (x0 + 1) % Width, y1 = (y0 + 1) % Height;

		int Width3 = Width * 3;

		BYTE* y0w = y0 * Width3 + Data;
		BYTE* y1w = y1 * Width3 + Data;

		int x03 = x0 * 3, x13 = x1 * 3;

		BYTE* a = y0w + x03;
		BYTE* b = y0w + x13;
		BYTE* c = y1w + x13;
		BYTE* d = y1w + x03;

		float u1 = dx - x0, v1 = dy - y0, u0 = 1.0f - u1, v0 = 1.0f - v1;

		u0 *= OD255;
		u1 *= OD255;

		float u0v0 = u0 * v0, u1v0 = u1 * v0, u1v1 = u1 * v1, u0v1 = u0 * v1;

		Color.r = u0v0 * a[0] + u1v0 * b[0] + u1v1 * c[0] + u0v1 * d[0];
		Color.g = u0v0 * a[1] + u1v0 * b[1] + u1v1 * c[1] + u0v1 * d[1];
		Color.b = u0v0 * a[2] + u1v0 * b[2] + u1v1 * c[2] + u0v1 * d[2];
	}

	return Color;
}

void CTexture::Destroy()
{
	if (Data != NULL)
	{
		delete[] Data;
		Data = NULL;
		Width = Height = 0;
	}
}