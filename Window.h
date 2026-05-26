#pragma once

#include <windows.h>

class CWnd
{
protected:
	const char* WindowName;
	HWND hWnd;
	HDC hDC;
	int Width, Height, Line;
	POINT LastCurPos;

public:
	CWnd();
	~CWnd();

	bool Create(HINSTANCE hInstance, const char* WindowName, int Width, int Height);
	void RePaint();
	void Show(bool Maximized = false);
	void MsgLoop();
	void Destroy();

	void OnKeyDown(UINT Key);
	void OnMouseMove(int cx, int cy);
	void OnMouseWheel(short zDelta);
	void OnPaint();
	void OnRButtonDown(int cx, int cy);
	void OnSize(int Width, int Height);
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);