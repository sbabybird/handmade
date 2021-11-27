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
#include <stdint.h>
#include "trace.h"
#define local_persist static
#define global_variable static
#define internal static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

// TODO: This is a global for now.
global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytePerPixel = 4;

internal void RenderWeirdGradient(int XOffset, int YOffset)
{
  int Width = BitmapWidth;
  int Height = BitmapHeight;

  int Pitch = BitmapWidth * BytePerPixel;
  uint8 *Row = (uint8 *)BitmapMemory;
  for (int Y = 0; Y < Height; ++Y)
  {
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < Width; ++X)
    {
      /*
        Pixel in memory is in BGRA format.
        LITTLE ENDIAN ARCITECTURE.
        Array of bytes is in memory in reverse order.
      */
      uint8 Blue = (uint8)(X + XOffset);
      uint8 Green = (uint8)(Y + YOffset);
      uint8 Red = 0;
      *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
    }
    Row += Pitch;
  }
}

internal void Win32ResizeDIBSection(int Width, int Height)
{
  if (BitmapMemory)
  {
    VirtualFree(BitmapMemory, 0, MEM_RELEASE);
  }

  BitmapWidth = Width;
  BitmapHeight = Height;

  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = BitmapWidth;
  BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;

  int BitmapMemorySize = BitmapWidth * BitmapHeight * BytePerPixel;
  BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

  // RenderWeirdGradient(0, 0);
}

internal void Win32UpdateWindow(HDC DeviceContext, RECT *WindowRect, int X, int Y, int Width, int Height)
{
  int WindowWidth = WindowRect->right - WindowRect->left;
  int WindowHeight = WindowRect->bottom - WindowRect->top;

  StretchDIBits(DeviceContext,
                X, Y, Width, Height,
                X, Y, Width, Height,
                BitmapMemory,
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
    RECT ClientRect;
    GetClientRect(hwnd, &ClientRect);
    Win32UpdateWindow(dc, &ClientRect, X, Y, Width, Height);
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
    HWND hwnd = CreateWindowEx(
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
      int XOffset = 0;
      int YOffset = 0;
      Running = true;
      while (Running)
      {
        MSG msg;
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
          if (msg.message == WM_QUIT)
          {
            Running = false;
          }

          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
        RenderWeirdGradient(XOffset, YOffset);

        HDC DeviceContext = GetDC(hwnd);
        RECT ClientRect;
        GetClientRect(hwnd, &ClientRect);
        Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top);
        ReleaseDC(hwnd, DeviceContext);

        ++XOffset;
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
