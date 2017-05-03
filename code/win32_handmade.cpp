// #include <windows.h>
// int CALLBACK
// WinMain(_In_  HINSTANCE hInstance,
//         _In_  HINSTANCE hPrevInstance,
//         _In_  LPSTR lpCmdLine,
//         _In_  int nCmdShow)
// {
//   MessageBox(0, "Hello world!", "hello", MB_OK|MB_ICONINFORMATION);
//   MessageBox(0, "new ", "new ", MB_ICONINFORMATION|MB_YESNO);
//   return 0;
// }

#include <windows.h>
#include "smj.h"
// TODO: reference additional headers your program requires here
#define local_persist static
#define global_variable static
#define internal static

// TODO(casey): This is a global for now.
global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void
Win32ResizeDIBSection(int Width, int Height)
{
  if (BitmapHandle) {
    DeleteObject(BitmapHandle);
  }

  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = Width;
  BitmapInfo.bmiHeader.biHeight = Height;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;

  if (BitmapDeviceContext) {
    BitmapDeviceContext = CreateCompatibleDC(0);
  }
  BitmapHandle = CreateDIBSection(
    BitmapDeviceContext, &BitmapInfo,
    DIB_RGB_COLORS,
    &BitmapMemory,
    0, 0);
}

internal void
Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
  StretchDIBits(DeviceContext,
    X, Y, Width, Height,
		X, Y, Width, Height,
    &BitmapMemory,
    &BitmapInfo,
    DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND hwnd,
UINT uMsg,
WPARAM wParam,
LPARAM lParam)
{
  LRESULT lrResult = 0;

  switch (uMsg)
  {
  case WM_SIZE:
  {
    RECT ClientRect;
    GetClientRect(hwnd, &ClientRect);
    int Width = ClientRect.right - ClientRect.left;
    int Height = ClientRect.bottom - ClientRect.top;
    Win32ResizeDIBSection(Width, Height);
    OutputDebugStringA("WM_SIZE\n");
  }	break;

  case WM_CLOSE:
  {
		// TODO(casey): Handle this with a message to the user?
		Running = false;
		OutputDebugStringA("WM_DESTROY\n");
	}	break;

	case WM_DESTROY:
	{	// TODO(casey): Handle this as an error - recreate window?
		Running = false;
		OutputDebugStringA("WM_CLOSE\n");
	}	break;

	case WM_ACTIVATEAPP:
	{
		OutputDebugStringA("WM_ACTIVATEAPP\n");
	}	break;

	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC dc = BeginPaint(hwnd, &Paint);
		int X = Paint.rcPaint.left;
		int Y = Paint.rcPaint.top;
		int Width = Paint.rcPaint.right - Paint.rcPaint.left;
		int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
		Win32UpdateWindow(dc, X, Y, Width, Height);
		EndPaint(hwnd, &Paint);
	} break;

	default:
	{
		OutputDebugStringA("default\n");
		lrResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}	break;
	}

	return lrResult;
}

CHello::CHello() {
   this->SayHello();
}

CHello::~CHello() {
  
}

void CHello::SayHello() {
  MessageBox(NULL, "hello", "hello", MB_OK);
}

CHelloA::CHelloA() {
  //this->SayHello();
}

CHelloA::~CHelloA() {
  
}

void CHelloA::SayHello() {
  MessageBox(NULL, "hello a", "hello a", MB_OK);
}

int CALLBACK
WinMain(_In_  HINSTANCE hInstance,
_In_  HINSTANCE hPrevInstance,
_In_  LPSTR lpCmdLine,
_In_  int nCmdShow)
{
	WNDCLASS wc = {};
  //CHelloA helloa; 
  CHelloA* p = new CHelloA();
  p->SayHello();

	// TODO(casey): Check if HREDRAW/VREDRAW/OWNDC still matter
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = Win32MainWindowCallback;
	wc.hInstance = hInstance;
	//wc.hIcon = ;
	wc.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&wc)) {
		HWND hwnd = CreateWindowExA(
			0,
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
			Running = true;
			while (Running) {
				MSG msg;
				BOOL rt = GetMessageA(&msg, 0, 0, 0);
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
			// TODO(casey): logging;
		}
	}
	else {
		// TODO(casey): Logging
	}

	return 0;
};

