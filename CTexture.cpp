#include "stdafx.h"

#include "CTexture.h"

#include "CString.h"

#pragma comment(lib, "Windowscodecs.lib")

extern CString ModuleDirectory, ErrorLog;
static const float OD255 = 1.0f / 255.0f;

namespace
{
	template <typename T>
	void SafeRelease(T*& ptr)
	{
		if (ptr != nullptr)
		{
			ptr->Release();
			ptr = nullptr;
		}
	}

	float WrapTextureCoord(float value)
	{
		value -= static_cast<float>(static_cast<int>(value));
		if (value < 0.0f) value += 1.0f;
		return value;
	}
}

CTexture::CTexture() : Width(0), Height(0)
{
}

CTexture::~CTexture()
{
}

bool CTexture::CreateTexture2D(const char* Texture2DFileName)
{
	CString FileName = ModuleDirectory + Texture2DFileName;
	CString ErrorText = "Error loading file " + FileName + "! -> ";

	int FileNameWLength = MultiByteToWideChar(CP_ACP, 0, FileName, -1, nullptr, 0);

	if (FileNameWLength <= 0)
	{
		ErrorLog.Append(ErrorText + "MultiByteToWideChar failed" + "\r\n");
		return false;
	}

	wchar_t* FileNameW = new wchar_t[static_cast<size_t>(FileNameWLength)];

	if (MultiByteToWideChar(CP_ACP, 0, FileName, -1, FileNameW, FileNameWLength) == 0)
	{
		delete[] FileNameW;
		ErrorLog.Append(ErrorText + "MultiByteToWideChar failed" + "\r\n");
		return false;
	}

	IWICImagingFactory* Factory = nullptr;
	IWICBitmapDecoder* Decoder = nullptr;
	IWICBitmapFrameDecode* Frame = nullptr;
	IWICFormatConverter* Converter = nullptr;

	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&Factory));

	if (FAILED(hr))
	{
		delete[] FileNameW;
		ErrorLog.Append(ErrorText + "CoCreateInstance failed" + "\r\n");
		return false;
	}

	hr = Factory->CreateDecoderFromFilename(FileNameW, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &Decoder);

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

	hr = Converter->Initialize(Frame, GUID_WICPixelFormat24bppBGR, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom);

	if (FAILED(hr))
	{
		SafeRelease(Converter);
		SafeRelease(Frame);
		SafeRelease(Decoder);
		SafeRelease(Factory);
		ErrorLog.Append(ErrorText + "Format conversion failed" + "\r\n");
		return false;
	}

	UINT texWidth = 0, texHeight = 0;
	hr = Converter->GetSize(&texWidth, &texHeight);

	if (FAILED(hr) || texWidth == 0 || texHeight == 0)
	{
		SafeRelease(Converter);
		SafeRelease(Frame);
		SafeRelease(Decoder);
		SafeRelease(Factory);
		ErrorLog.Append(ErrorText + "Width or Height is 0" + "\r\n");
		return false;
	}

	Destroy();

	Width = (int)texWidth;
	Height = (int)texHeight;

	UINT Pitch = texWidth * 3;
	UINT BufferSize = Pitch * texHeight;
	Data.resize(BufferSize);

	std::vector<BYTE> Bits(BufferSize);

	hr = Converter->CopyPixels(nullptr, Pitch, BufferSize, Bits.data());

	if (FAILED(hr))
	{
		Destroy();
		SafeRelease(Converter);
		SafeRelease(Frame);
		SafeRelease(Decoder);
		SafeRelease(Factory);
		ErrorLog.Append(ErrorText + "CopyPixels failed" + "\r\n");
		return false;
	}

	BYTE* data = Data.data();
	const BYTE* line = Bits.data();

	for (int y = 0; y < Height; y++)
	{
		const BYTE* pixel = line;

		for (int x = 0; x < Width; x++)
		{
			data[0] = pixel[2];
			data[1] = pixel[1];
			data[2] = pixel[0];

			pixel += 3;
			data += 3;
		}

		line += Pitch;
	}

	SafeRelease(Converter);
	SafeRelease(Frame);
	SafeRelease(Decoder);
	SafeRelease(Factory);

	return true;
}

Vector3 CTexture::GetColorNearest(float s, float t)
{
	Vector3 Color = Vector3(1.0f);

	if (!Data.empty())
	{
		s = WrapTextureCoord(s);
		t = WrapTextureCoord(t);

		int x = (int)(s * static_cast<float>(Width)), y = (int)(t * static_cast<float>(Height));

		const BYTE* data = (Width * y + x) * 3 + Data.data();

		Color.r = OD255 * data[0];
		Color.g = OD255 * data[1];
		Color.b = OD255 * data[2];
	}

	return Color;
}

Vector3 CTexture::GetColorBilinear(float s, float t)
{
	Vector3 Color = Vector3(1.0f);

	if (!Data.empty())
	{
		s = WrapTextureCoord(s);
		t = WrapTextureCoord(t);

		float dx = s * static_cast<float>(Width) - 0.5f, dy = t * static_cast<float>(Height) - 0.5f;

		if (dx < 0.0f) dx += static_cast<float>(Width);
		if (dy < 0.0f) dy += static_cast<float>(Height);

		int x0 = (int)dx, y0 = (int)dy, x1 = (x0 + 1) % Width, y1 = (y0 + 1) % Height;

		int Width3 = Width * 3;

		const BYTE* y0w = y0 * Width3 + Data.data();
		const BYTE* y1w = y1 * Width3 + Data.data();

		int x03 = x0 * 3, x13 = x1 * 3;

		const BYTE* a = y0w + x03;
		const BYTE* b = y0w + x13;
		const BYTE* c = y1w + x13;
		const BYTE* d = y1w + x03;

		float u1 = dx - static_cast<float>(x0), v1 = dy - static_cast<float>(y0), u0 = 1.0f - u1, v0 = 1.0f - v1;

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
	Data.clear();
	Data.shrink_to_fit();
	Width = Height = 0;
}