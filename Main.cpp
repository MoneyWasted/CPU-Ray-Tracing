#include "stdafx.h"

#include "CWindow.h"
#include "CApplication.h"
#include "CString.h"

extern CWindow Wnd;

CString ModuleDirectory, ErrorLog;

int APIENTRY WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

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