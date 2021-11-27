/**
 * @file win32_handmade.cpp
 * @author SunMinjie (smj@ieforever.com)
 * @brief
 * @version 0.1
 * @date 2021-11-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <windows.h>
#include "trace.h"
#define local_persist static
#define global_variable static
#define internal static

// TODO: This is a global for now.
global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void Win32ResizeDIBSection(int Width, int Height)
{
  if (BitmapHandle)
  {
    DeleteObject(BitmapHandle);
  }

  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = Width;
  BitmapInfo.bmiHeader.biHeight = Height;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;

  if (!BitmapDeviceContext)
  {
    BitmapDeviceContext = CreateCompatibleDC(0);
  }
  BitmapHandle = CreateDIBSection(
      BitmapDeviceContext, &BitmapInfo,
      DIB_RGB_COLORS,
      &BitmapMemory,
      0, 0);
}

internal void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
  StretchDIBits(DeviceContext,
                X, Y, Width, Height,
                X, Y, Width, Height,
                &BitmapMemory,
                &BitmapInfo,
                DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
    TRACE("WM_SIZE: %d, %d\n", Width, Height);
  }
  break;

  case WM_CLOSE:
  {
    // TODO: Handle this with a message to the user?
    Running = false;
    TRACE("WM_CLOSE\n");
  }
  break;

  case WM_DESTROY:
  {
    // TODO: Handle this as an error - recreate window?
    Running = false;
    TRACE("WM_DESTROY\n");
  }
  break;

  case WM_ACTIVATEAPP:
  {
    TRACE("WM_ACTIVATEAPP\n");
  }
  break;

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
  }
  break;

  default:
  {
    TRACE("default\n");
    lrResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
  break;
  }

  return lrResult;
}

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
  WNDCLASS wc = {};

  // TODO: Check if HREDRAW/VREDRAW/OWNDC still matter
  wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = Win32MainWindowCallback;
  wc.hInstance = hInstance;
  // wc.hIcon = ;
  wc.lpszClassName = "HandmadeHeroWindowClass";

  if (RegisterClass(&wc))
  {
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

    if (hwnd)
    {
      Running = true;
      while (Running)
      {
        MSG msg;
        BOOL rt = GetMessageA(&msg, 0, 0, 0);
        if (rt > 0)
        {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
        else
        {
          break;
        }
      }
    }
    else
    {
      // TODO: Logging;
    }
  }
  else
  {
    // TODO: Logging
  }

  return 0;
};
