#pragma once

#include <d3d11.h>
#include <dxgi.h>

class CWindow
{
private:
	const char* WindowName;
	HWND hWnd;
	int Width, Height, Line;
	POINT LastCurPos;
	ID3D11Device* D3DDevice;
	ID3D11DeviceContext* D3DDeviceContext;
	IDXGISwapChain* SwapChain;
	ID3D11RenderTargetView* BackBufferRTV;
	ID3D11Texture2D* SwapChainBackBuffer;

	void OnKeyDown(UINT Key);
	void OnMouseMove(int cx, int cy);
	void OnMouseWheel(short zDelta);
	void OnPaint();
	void OnRButtonDown(int cx, int cy);
	void OnSize(int Width, int Height);
	bool InitD3D();
	void DestroyD3D();

	friend LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);

public:
	CWindow();
	~CWindow();

	bool Create(HINSTANCE hInstance, const char* windowName, int width, int height);
	void RePaint();
	void Show(bool Maximized = false);
	void MsgLoop();
	void Destroy();

	// Expose device/context to other modules
	ID3D11Device* GetD3DDevice() { return D3DDevice; }
	ID3D11DeviceContext* GetD3DDeviceContext() { return D3DDeviceContext; }
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);