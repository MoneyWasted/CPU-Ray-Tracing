#include "stdafx.h"

#include "CWindow.h"

#include "CString.h"
#include "CCamera.h"
#include "CApplication.h"

extern CString ModuleDirectory, ErrorLog;
extern CCamera Camera;
extern CApplication Application;

CWindow::CWindow()
{
	char* moduledirectory = new char[MAX_PATH];
	GetModuleFileNameA(GetModuleHandleA(NULL), moduledirectory, MAX_PATH);
	*(strrchr(moduledirectory, '\\') + 1) = 0;
	ModuleDirectory = moduledirectory;
	delete[] moduledirectory;
}

CWindow::~CWindow()
{
}

bool CWindow::Create(HINSTANCE hInstance, const char* WindowName, int Width, int Height)
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
	WndClassEx.lpszClassName = L"Win32CPUApplicationWindow";

	if (RegisterClassEx(&WndClassEx) == 0)
	{
		ErrorLog.Set("RegisterClassEx failed!");
		return false;
	}

	this->WindowName = WindowName;

	this->Width = Width;
	this->Height = Height;

	DWORD Style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if ((hWnd = CreateWindowExA(WS_EX_APPWINDOW, "Win32CPUApplicationWindow", WindowName, Style, 0, 0, Width, Height, NULL, NULL, hInstance, NULL)) == NULL)
	{
		ErrorLog.Set("CreateWindowExA failed!");
		return false;
	}

	if ((hDC = GetDC(hWnd)) == NULL)
	{
		ErrorLog.Set("GetDC failed!");
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

	DestroyWindow(hWnd);
}

void CWindow::OnKeyDown(UINT Key)
{
	switch (Key)
	{
	case '1':
		if (Application.SetSamples(1)) RePaint();
		break;

	case '2':
		if (Application.SetSamples(2)) RePaint();
		break;

	case '3':
		if (Application.SetSamples(3)) RePaint();
		break;

	case '4':
		if (Application.SetSamples(4)) RePaint();
		break;

	case VK_F1:
		Application.Textures = !Application.Textures;
		RePaint();
		break;

	case VK_F2:
		Application.SoftShadows = !Application.SoftShadows;
		RePaint();
		break;

	case VK_F3:
		Application.AmbientOcclusion = !Application.AmbientOcclusion;
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
	static bool RayTracing;

	if (Line == 0)
	{
		Application.ClearColorBuffer();

		Start = GetTickCount64();

		RayTracing = true;
	}

	ULONGLONG start = GetTickCount64();

	while (Line < Height && GetTickCount64() - start < 250)
	{
		Application.RayTrace(Line++);
	}

	Application.SwapBuffers(hDC);

	if (RayTracing)
	{
		if (Line == Height)
		{
			RayTracing = false;
			Application.MapHDRColors();
		}

		ULONGLONG End = GetTickCount64();

		CString text = WindowName;

		text.Append(" - %dx%d", Width, Height);
		text.Append(", Supersampling %dx", Application.GetSamples());
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

void CWindow::OnSize(int Width, int Height)
{
	this->Width = Width;
	this->Height = Height;

	Application.Resize(Width, Height);

	RePaint();
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