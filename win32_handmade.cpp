/**
 * @file win32_handmade.cpp
 * @author SunMinjie (smj@ieforever.com)
 * @brief 跟着Youtube上的handmadehero系列编程课程的练习
 * @link https://www.youtube.com/watch?v=w7ay7QXmo_o&t=2756s
 * @version 0.5
 * @init_date 2015-05-10
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

struct win32_offscreen_buffer
{
  BITMAPINFO Info;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};
global_variable win32_offscreen_buffer GlobalBackBuffer;

struct win32_window_dimension
{
  int Width;
  int Height;
};

win32_window_dimension Win32GetWindowDimension(HWND Window)
{
  win32_window_dimension Result;
  RECT ClientRect;
  GetClientRect(Window, &ClientRect);
  Result.Width = ClientRect.right - ClientRect.left;
  Result.Height = ClientRect.bottom - ClientRect.top;
  return Result;
}

internal void RenderWeirdGradient(win32_offscreen_buffer buffer, int XOffset, int YOffset)
{
  uint8 *Row = (uint8 *)(buffer.Memory);
  for (int Y = 0; Y < buffer.Height; ++Y)
  {
    uint32 *Pixel = (uint32 *)Row;
    for (int X = 0; X < buffer.Width; ++X)
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
    Row += buffer.Pitch;
  }
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int Width, int Height)
{
  if (buffer->Memory)
  {
    VirtualFree(buffer->Memory, 0, MEM_RELEASE);
  }

  buffer->Width = Width;
  buffer->Height = Height;
  buffer->BytesPerPixel = 4;

  buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
  buffer->Info.bmiHeader.biWidth = buffer->Width;
  buffer->Info.bmiHeader.biHeight = -buffer->Height;
  buffer->Info.bmiHeader.biPlanes = 1;
  buffer->Info.bmiHeader.biBitCount = 32;
  buffer->Info.bmiHeader.biCompression = BI_RGB;

  int BitmapMemorySize = buffer->Width * buffer->Height * buffer->BytesPerPixel;
  buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  buffer->Pitch = buffer->Width * buffer->BytesPerPixel;
}

internal void Win32DisplayBufferInWindow(HDC DeviceContext, win32_offscreen_buffer *Buffer, int WindowWidth, int WindowHeight)
{
  StretchDIBits(DeviceContext,
                0, 0, WindowWidth, WindowHeight,
                0, 0, Buffer->Width, Buffer->Height,
                Buffer->Memory,
                &Buffer->Info,
                DIB_RGB_COLORS,
                SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  LRESULT lrResult = 0;

  switch (uMsg)
  {
  case WM_SIZE:
  {
    win32_window_dimension Dimension = Win32GetWindowDimension(hwnd);
    TRACE("WM_SIZE: %d, %d\n", Dimension.Width, Dimension.Height);
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
    win32_window_dimension Dimension = Win32GetWindowDimension(hwnd);
    Win32DisplayBufferInWindow(dc, &GlobalBackBuffer, Dimension.Width, Dimension.Height);
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
  Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

  WNDCLASS wc = {};

  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = Win32MainWindowCallback;
  wc.hInstance = hInstance;
  // wc.hIcon = ;
  wc.lpszClassName = "HandmadeHeroWindowClass";

  if (RegisterClass(&wc))
  {
    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, "handmade",
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                               0, 0, hInstance, 0);

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
        RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);

        HDC DeviceContext = GetDC(hwnd);
        win32_window_dimension Dimension = Win32GetWindowDimension(hwnd);
        Win32DisplayBufferInWindow(DeviceContext, &GlobalBackBuffer, Dimension.Width, Dimension.Height);
        ReleaseDC(hwnd, DeviceContext);

        ++XOffset;
        YOffset += 2;
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
