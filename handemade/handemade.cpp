#include"stdafx.h"
#include<windows.h>

LRESULT CALLBACK
MainWindowCallback(HWND hwnd,
UINT uMsg,
WPARAM wParam,
LPARAM lParam)
{
	LRESULT lrResult = 0;

	switch (uMsg) {
	case WM_SIZE:
		OutputDebugStringA("WM_SIZE\n");
		break;

	case WM_DESTROY:
		OutputDebugStringA("WM_DESTROY\n");
		break;

	case WM_CLOSE:
		OutputDebugStringA("WM_CLOSE\n");
		exit(0);
		break;

	case WM_ACTIVATEAPP:
		OutputDebugStringA("WM_ACTIVATEAPP\n");
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC dc = BeginPaint(hwnd, &Paint);
		int X = Paint.rcPaint.left;
		int Y = Paint.rcPaint.top;
		int Width = Paint.rcPaint.right - Paint.rcPaint.left;
		int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
		static DWORD op = WHITENESS;
		PatBlt(dc, X, Y, Width, Height, op);
		op = (op == WHITENESS) ? BLACKNESS : WHITENESS;
		EndPaint(hwnd, &Paint);
	} break;

	default:
		OutputDebugStringA("default\n");
		lrResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
		break;
	}

	return lrResult;
}

int CALLBACK
WinMain(_In_  HINSTANCE hInstance,
_In_  HINSTANCE hPrevInstance,
_In_  LPSTR lpCmdLine,
_In_  int nCmdShow)
{
	MessageBox(0, "message", "title",
		MB_OK | MB_ICONINFORMATION);

	WNDCLASS wc = {};

	// TODO(casey): Check if HREDRAW/VREDRAW/OWNDC still matter
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWindowCallback;
	wc.hInstance = hInstance;
	//wc.hIcon = ;
	wc.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&wc)) {
		HWND hwnd = CreateWindowEx(0,
			wc.lpszClassName,
			"handmade",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			hInstance,
			0);

		if (hwnd) {
			for (;;) {
				MSG msg;
				BOOL rt = GetMessage(&msg, 0, 0, 0);
				if (rt > 0) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else {
					break;
				}
			}
		}
		else {
			// TODO: logging;
		}
	}
	else {
		// TODO(casey): Logging
	}

	return 0;
};

