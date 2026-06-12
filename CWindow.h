#pragma once

class CWindow
{
private:
	const char* WindowName;
	HWND hWnd;
	HDC hDC;
	int Width, Height, Line;
	POINT LastCurPos;

	void OnKeyDown(UINT Key);
	void OnMouseMove(int cx, int cy);
	void OnMouseWheel(short zDelta);
	void OnPaint();
	void OnRButtonDown(int cx, int cy);
	void OnSize(int Width, int Height);

	friend LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);

public:
	CWindow();
	~CWindow();

	bool Create(HINSTANCE hInstance, const char* windowName, int width, int height);
	void RePaint();
	void Show(bool Maximized = false);
	void MsgLoop();
	void Destroy();
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);