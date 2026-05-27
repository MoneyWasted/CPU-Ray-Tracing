#include "stdafx.h"
#include "Window.h"
#include "MyRayTracer.h"

extern CWnd Wnd;

CString ModuleDirectory, ErrorLog;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdLine, int iShow)
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if (FAILED(hr))
	{
		MessageBoxA(NULL, "CoInitializeEx failed!", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	if (Wnd.Create(hInstance, "CPU Ray Tracer", 800, 600))
	{
		Wnd.Show();
		Wnd.MsgLoop();
	}
	else
	{
		MessageBoxA(NULL, ErrorLog, "Error", MB_OK | MB_ICONERROR);
	}

	Wnd.Destroy();
	CoUninitialize();

	return 0;
}