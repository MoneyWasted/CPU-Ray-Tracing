#include "stdafx.h"

#include "CWindow.h"

#include "CString.h"
#include "CCamera.h"
#include "CApplication.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

extern CString ModuleDirectory, ErrorLog;
extern CCamera Camera;
extern CApplication Application;

CWindow::CWindow() :
	WindowName(nullptr),
	hWnd(nullptr),
	Width(0), Height(0), Line(0),
	LastCurPos{},
	D3DDevice(nullptr),
	D3DDeviceContext(nullptr),
	SwapChain(nullptr),
	BackBufferRTV(nullptr),
	SwapChainBackBuffer(nullptr)
{
	char moduledirectory[MAX_PATH];
	GetModuleFileNameA(GetModuleHandleA(nullptr), moduledirectory, MAX_PATH);
	*(strrchr(moduledirectory, '\\') + 1) = 0;
	ModuleDirectory = moduledirectory;
}

CWindow::~CWindow()
{
}

bool CWindow::Create(HINSTANCE hInstance, const char* windowName, int width, int height)
{
	WNDCLASSEX WndClassEx;

	memset(&WndClassEx, 0, sizeof(WNDCLASSEX));

	WndClassEx.cbSize = sizeof(WNDCLASSEX);
	WndClassEx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClassEx.lpfnWndProc = WndProc;
	WndClassEx.hInstance = hInstance;
	WndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClassEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClassEx.lpszClassName = L"Win32GPUApplicationWindow";

	if (RegisterClassEx(&WndClassEx) == 0)
	{
		ErrorLog.Set("RegisterClassEx failed!");
		return false;
	}

	WindowName = windowName;
	Width = width;
	Height = height;

	DWORD Style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if ((hWnd = CreateWindowExA(WS_EX_APPWINDOW, "Win32GPUApplicationWindow", windowName, Style, 0, 0, width, height, NULL, NULL, hInstance, NULL)) == NULL)
	{
		ErrorLog.Set("CreateWindowExA failed!");
		return false;
	}

	if (!InitD3D())
	{
		if (((char*)ErrorLog)[0] == 0)
		{
			ErrorLog.Set("Direct3D 11 initialization failed!");
		}
		return false;
	}

	return Application.Init();
}

void CWindow::RePaint()
{
	Line = 0;
	InvalidateRect(hWnd, NULL, FALSE);
}

void CWindow::Show(bool Maximized)
{
	RECT dRect, wRect, cRect;

	GetWindowRect(GetDesktopWindow(), &dRect);
	GetWindowRect(hWnd, &wRect);
	GetClientRect(hWnd, &cRect);

	wRect.right += Width - cRect.right;
	wRect.bottom += Height - cRect.bottom;

	wRect.right -= wRect.left;
	wRect.bottom -= wRect.top;

	wRect.left = dRect.right / 2 - wRect.right / 2;
	wRect.top = dRect.bottom / 2 - wRect.bottom / 2;

	MoveWindow(hWnd, wRect.left, wRect.top, wRect.right, wRect.bottom, FALSE);

	ShowWindow(hWnd, Maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
}

void CWindow::MsgLoop()
{
	MSG Msg;

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}

void CWindow::Destroy()
{
	Application.Destroy();
	DestroyD3D();

	DestroyWindow(hWnd);
}

void CWindow::OnKeyDown(UINT Key)
{
	switch (Key)
	{
	case '1':
		if (Application.SetSamples(1)) { Application.MarkSceneDirty(); RePaint(); }
		break;

	case '2':
		if (Application.SetSamples(2)) { Application.MarkSceneDirty(); RePaint(); }
		break;

	case '3':
		if (Application.SetSamples(3)) { Application.MarkSceneDirty(); RePaint(); }
		break;

	case '4':
		if (Application.SetSamples(4)) { Application.MarkSceneDirty(); RePaint(); }
		break;

	case VK_F1:
		Application.Textures = !Application.Textures;
		Application.MarkSceneDirty();
		RePaint();
		break;

	case VK_F2:
		Application.SoftShadows = !Application.SoftShadows;
		Application.MarkSceneDirty();
		RePaint();
		break;

	case VK_F3:
		Application.AmbientOcclusion = !Application.AmbientOcclusion;
		Application.MarkSceneDirty();
		RePaint();
		break;
	}

	if (Camera.OnKeyDown(Key))
	{
		RePaint();
	}
}

void CWindow::OnMouseMove(int cx, int cy)
{
	if (GetKeyState(VK_RBUTTON) & 0x80)
	{
		Camera.OnMouseMove(LastCurPos.x - cx, LastCurPos.y - cy);

		LastCurPos.x = cx;
		LastCurPos.y = cy;

		RePaint();
	}
}

void CWindow::OnMouseWheel(short zDelta)
{
	Camera.OnMouseWheel(zDelta);

	RePaint();
}

void CWindow::OnPaint()
{
	PAINTSTRUCT ps;

	BeginPaint(hWnd, &ps);

	static ULONGLONG Start;
	static bool FramePending;
	static bool GPUFrame;

	if (Line == 0)
	{
		if (!Application.HasGPUBackend())
		{
			Application.ClearColorBuffer();
		}
		Start = GetTickCount64();
		FramePending = true;
		GPUFrame = false;
	}

	if (Application.HasGPUBackend())
	{
		if (Application.RenderGPU(Camera) && SwapChainBackBuffer)
		{
			Application.PresentGPU(SwapChainBackBuffer);
			FramePending = false;
			GPUFrame = true;
			Line = Height;
		}
	}
	else
	{
		ULONGLONG start = GetTickCount64();
		while (Line < Height && GetTickCount64() - start < 250)
		{
			Application.RayTrace(Line++);
		}
	}

	if (SwapChain)
	{
		SwapChain->Present(1, 0);
	}

	if (GPUFrame)
	{
		FramePending = false;

		ULONGLONG End = GetTickCount64();

		CString text = WindowName;
		text.Append(" - %dx%d", Width, Height);
		text.Append(", GPU Primary Path");
		text.Append(", Time: %.03f s", (float)(End - Start) * 0.001f);

		SetWindowTextA(hWnd, text);
		GPUFrame = false;
	}
	else if (FramePending)
	{
		if (Line == Height)
		{
			FramePending = false;
			Application.MapHDRColors();
		}

		ULONGLONG End = GetTickCount64();

		CString text = WindowName;

		text.Append(" - %dx%d", Width, Height);
		text.Append(", Path Tracing");
		text.Append(", Time: %.03f s", (float)(End - Start) * 0.001f);

		SetWindowTextA(hWnd, text);

		InvalidateRect(hWnd, NULL, FALSE);
	}

	EndPaint(hWnd, &ps);
}

void CWindow::OnRButtonDown(int cx, int cy)
{
	LastCurPos.x = cx;
	LastCurPos.y = cy;
}

void CWindow::OnSize(int width, int height)
{
	Width = width;
	Height = height;

	if (SwapChain && D3DDeviceContext)
	{
		if (BackBufferRTV)
		{
			BackBufferRTV->Release();
			BackBufferRTV = nullptr;
		}
		if (SwapChainBackBuffer)
		{
			SwapChainBackBuffer->Release();
			SwapChainBackBuffer = nullptr;
		}

		D3DDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
		if (FAILED(SwapChain->ResizeBuffers(0, Width > 0 ? static_cast<UINT>(Width) : 1u, Height > 0 ? static_cast<UINT>(Height) : 1u, DXGI_FORMAT_UNKNOWN, 0)))
		{
			Application.Resize(width, height);
			RePaint();
			return;
		}

		if (SUCCEEDED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&SwapChainBackBuffer))))
		{
			D3DDevice->CreateRenderTargetView(SwapChainBackBuffer, nullptr, &BackBufferRTV);
		}
	}

	Application.Resize(width, height);
	Application.MarkSceneDirty();

	RePaint();
}

bool CWindow::InitD3D()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = Width > 0 ? static_cast<UINT>(Width) : 1u;
	swapChainDesc.BufferDesc.Height = Height > 0 ? static_cast<UINT>(Height) : 1u;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createFlags = 0;
#ifdef _DEBUG
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	const D3D_FEATURE_LEVEL levels11[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	const D3D_FEATURE_LEVEL levels10[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
	D3D_FEATURE_LEVEL selectedLevel{};

	auto TryCreateDevice = [&](D3D_DRIVER_TYPE driverType, UINT flags, const D3D_FEATURE_LEVEL* levels, UINT levelCount) -> HRESULT
	{
		if (SwapChain)
		{
			SwapChain->Release();
			SwapChain = nullptr;
		}
		if (D3DDeviceContext)
		{
			D3DDeviceContext->Release();
			D3DDeviceContext = nullptr;
		}
		if (D3DDevice)
		{
			D3DDevice->Release();
			D3DDevice = nullptr;
		}

		return D3D11CreateDeviceAndSwapChain(
			nullptr,
			driverType,
			nullptr,
			flags,
			levels,
			levelCount,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&SwapChain,
			&D3DDevice,
			&selectedLevel,
			&D3DDeviceContext);
	};

	HRESULT hr = TryCreateDevice(D3D_DRIVER_TYPE_HARDWARE, createFlags, levels11, ARRAYSIZE(levels11));

	if (hr == E_INVALIDARG)
	{
		hr = TryCreateDevice(D3D_DRIVER_TYPE_HARDWARE, createFlags, levels10, ARRAYSIZE(levels10));
	}

	if (FAILED(hr) && (createFlags & D3D11_CREATE_DEVICE_DEBUG) && hr == DXGI_ERROR_SDK_COMPONENT_MISSING)
	{
		hr = TryCreateDevice(D3D_DRIVER_TYPE_HARDWARE, 0, levels11, ARRAYSIZE(levels11));
		if (hr == E_INVALIDARG)
		{
			hr = TryCreateDevice(D3D_DRIVER_TYPE_HARDWARE, 0, levels10, ARRAYSIZE(levels10));
		}
	}

	if (FAILED(hr))
	{
		hr = TryCreateDevice(D3D_DRIVER_TYPE_WARP, createFlags, levels11, ARRAYSIZE(levels11));
		if (hr == E_INVALIDARG)
		{
			hr = TryCreateDevice(D3D_DRIVER_TYPE_WARP, createFlags, levels10, ARRAYSIZE(levels10));
		}

		if (FAILED(hr) && (createFlags & D3D11_CREATE_DEVICE_DEBUG) && hr == DXGI_ERROR_SDK_COMPONENT_MISSING)
		{
			hr = TryCreateDevice(D3D_DRIVER_TYPE_WARP, 0, levels11, ARRAYSIZE(levels11));
			if (hr == E_INVALIDARG)
			{
				hr = TryCreateDevice(D3D_DRIVER_TYPE_WARP, 0, levels10, ARRAYSIZE(levels10));
			}
		}

		if (FAILED(hr))
		{
			ErrorLog.Set("D3D11 init failed: device creation failed (HRESULT=0x%08X)", static_cast<unsigned int>(hr));
			return false;
		}
	}

	if (FAILED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&SwapChainBackBuffer))))
	{
		ErrorLog.Set("D3D11 init failed: unable to acquire swap chain back buffer");
		DestroyD3D();
		return false;
	}

	if (FAILED(D3DDevice->CreateRenderTargetView(SwapChainBackBuffer, nullptr, &BackBufferRTV)))
	{
		ErrorLog.Set("D3D11 init failed: unable to create render target view");
		DestroyD3D();
		return false;
	}

	// No staging texture needed for GPU presentation.

	return true;
}

void CWindow::DestroyD3D()
{
	if (BackBufferRTV)
	{
		BackBufferRTV->Release();
		BackBufferRTV = nullptr;
	}
	if (SwapChainBackBuffer)
	{
		SwapChainBackBuffer->Release();
		SwapChainBackBuffer = nullptr;
	}
	if (SwapChain)
	{
		SwapChain->Release();
		SwapChain = nullptr;
	}
	if (D3DDeviceContext)
	{
		D3DDeviceContext->Release();
		D3DDeviceContext = nullptr;
	}
	if (D3DDevice)
	{
		D3DDevice->Release();
		D3DDevice = nullptr;
	}
}

CWindow Wnd;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_MOUSEMOVE:
		Wnd.OnMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_MOUSEWHEEL:
		Wnd.OnMouseWheel(static_cast<short>(HIWORD(wParam)));
		break;

	case WM_KEYDOWN:
		Wnd.OnKeyDown((UINT)wParam);
		break;

	case WM_PAINT:
		Wnd.OnPaint();
		break;

	case WM_RBUTTONDOWN:
		Wnd.OnRButtonDown(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_SIZE:
		Wnd.OnSize(LOWORD(lParam), HIWORD(lParam));
		break;

	default:
		return DefWindowProc(hWnd, uiMsg, wParam, lParam);
	}

	return 0;
}