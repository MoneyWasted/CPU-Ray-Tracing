#include "stdafx.h"
#include "Window.h"
#include "MyRayTracer.h"

extern CWnd Wnd;

CString ModuleDirectory, ErrorLog;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdLine, int iShow)
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

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

	return 0;
}