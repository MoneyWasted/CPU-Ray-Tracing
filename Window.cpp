#include "stdafx.h"

#include "Window.h"
#include "Camera.h"
#include "MyRayTracer.h"

extern CString ModuleDirectory, ErrorLog;
extern CCamera Camera;
extern CMyRayTracer RayTracer;

CWnd::CWnd()
{
	char* moduledirectory = new char[MAX_PATH];
	GetModuleFileNameA(GetModuleHandleA(NULL), moduledirectory, MAX_PATH);
	*(strrchr(moduledirectory, '\\') + 1) = 0;
	ModuleDirectory = moduledirectory;
	delete[] moduledirectory;
}

CWnd::~CWnd()
{
}

bool CWnd::Create(HINSTANCE hInstance, const char* WindowName, int Width, int Height)
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
	WndClassEx.lpszClassName = L"Win32CPURayTracerWindow";

	if (RegisterClassEx(&WndClassEx) == 0)
	{
		ErrorLog.Set("RegisterClassEx failed!");
		return false;
	}

	this->WindowName = WindowName;

	this->Width = Width;
	this->Height = Height;

	DWORD Style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if ((hWnd = CreateWindowExA(WS_EX_APPWINDOW, "Win32CPURayTracerWindow", WindowName, Style, 0, 0, Width, Height, NULL, NULL, hInstance, NULL)) == NULL)
	{
		ErrorLog.Set("CreateWindowExA failed!");
		return false;
	}

	if ((hDC = GetDC(hWnd)) == NULL)
	{
		ErrorLog.Set("GetDC failed!");
		return false;
	}

	return RayTracer.Init();
}

void CWnd::RePaint()
{
	Line = 0;
	InvalidateRect(hWnd, NULL, FALSE);
}

void CWnd::Show(bool Maximized)
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

void CWnd::MsgLoop()
{
	MSG Msg;

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}

void CWnd::Destroy()
{
	RayTracer.Destroy();

	DestroyWindow(hWnd);
}

void CWnd::OnKeyDown(UINT Key)
{
	switch (Key)
	{
	case '1':
		if (RayTracer.SetSamples(1)) RePaint();
		break;

	case '2':
		if (RayTracer.SetSamples(2)) RePaint();
		break;

	case '3':
		if (RayTracer.SetSamples(3)) RePaint();
		break;

	case '4':
		if (RayTracer.SetSamples(4)) RePaint();
		break;

	case VK_F1:
		RayTracer.Textures = !RayTracer.Textures;
		RePaint();
		break;

	case VK_F2:
		RayTracer.SoftShadows = !RayTracer.SoftShadows;
		RePaint();
		break;

	case VK_F3:
		RayTracer.AmbientOcclusion = !RayTracer.AmbientOcclusion;
		RePaint();
		break;
	}

	if (Camera.OnKeyDown(Key))
	{
		RePaint();
	}
}

void CWnd::OnMouseMove(int cx, int cy)
{
	if (GetKeyState(VK_RBUTTON) & 0x80)
	{
		Camera.OnMouseMove(LastCurPos.x - cx, LastCurPos.y - cy);

		LastCurPos.x = cx;
		LastCurPos.y = cy;

		RePaint();
	}
}

void CWnd::OnMouseWheel(short zDelta)
{
	Camera.OnMouseWheel(zDelta);

	RePaint();
}

void CWnd::OnPaint()
{
	PAINTSTRUCT ps;

	BeginPaint(hWnd, &ps);

	static DWORD Start;
	static bool RayTracing;

	if (Line == 0)
	{
		RayTracer.ClearColorBuffer();

		Start = GetTickCount();

		RayTracing = true;
	}

	DWORD start = GetTickCount();

	while (Line < Height && GetTickCount() - start < 250)
	{
		RayTracer.RayTrace(Line++);
	}

	RayTracer.SwapBuffers(hDC);

	if (RayTracing)
	{
		if (Line == Height)
		{
			RayTracing = false;
			RayTracer.MapHDRColors();
		}

		DWORD End = GetTickCount();

		CString text = WindowName;

		text.Append(" - %dx%d", Width, Height);
		text.Append(", Supersampling %dx", RayTracer.GetSamples());
		text.Append(", Time: %.03f s", (float)(End - Start) * 0.001f);

		SetWindowTextA(hWnd, text);

		InvalidateRect(hWnd, NULL, FALSE);
	}

	EndPaint(hWnd, &ps);
}

void CWnd::OnRButtonDown(int cx, int cy)
{
	LastCurPos.x = cx;
	LastCurPos.y = cy;
}

void CWnd::OnSize(int Width, int Height)
{
	this->Width = Width;
	this->Height = Height;

	RayTracer.Resize(Width, Height);

	RePaint();
}

CWnd Wnd;

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

	case 0x020A: // WM_MOUSWHEEL
		Wnd.OnMouseWheel(HIWORD(wParam));
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