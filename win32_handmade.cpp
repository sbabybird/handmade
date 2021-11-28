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
#include <Xinput.h>
#include <dsound.h>
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

// 关闭C4800警告 （将int类型强制转换为bool后）
#pragma warning(disable : 4800)

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

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
  return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
  return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void Win32LoadXInput(void)
{
  HMODULE XInputLibrary = LoadLibrary("xinput1_4.dll");
  if (!XInputLibrary)
  {
    XInputLibrary = LoadLibrary("xinput1_3.dll");
  }
  if (XInputLibrary)
  {
    XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
    XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
  }
}

internal void Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
  HMODULE DSoundLibrary = LoadLibrary("dsound.dll");
  if (DSoundLibrary)
  {
    direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
    LPDIRECTSOUND DirectSound;
    if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
    {
      WAVEFORMATEX WaveFormat = {};
      WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
      WaveFormat.nChannels = 2;
      WaveFormat.nSamplesPerSec = SamplesPerSecond;
      WaveFormat.wBitsPerSample = 16;
      WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
      WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
      WaveFormat.cbSize = 0;

      if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
      {
        DSBUFFERDESC BufferDescription = {};
        BufferDescription.dwSize = sizeof(BufferDescription);
        BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

        LPDIRECTSOUNDBUFFER PrimaryBuffer;
        if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
        {
          if (SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
          {
            OutputDebugStringA("Primary buffer format was set.\n");
          }
          else
          {
            OutputDebugStringA("Failed to set primary buffer format.\n");
          }
        }
        else
        {
          OutputDebugStringA("Failed to create primary buffer.\n");
        }
      }
      else
      {
        OutputDebugStringA("Failed to set cooperative level.\n");
      }

      DSBUFFERDESC BufferDescription = {};
      BufferDescription.dwSize = sizeof(BufferDescription);
      BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
      BufferDescription.dwBufferBytes = BufferSize;
      BufferDescription.lpwfxFormat = &WaveFormat;
      LPDIRECTSOUNDBUFFER SecondaryBuffer;
      if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &SecondaryBuffer, 0)))
      {
        OutputDebugStringA("Secondary buffer created.\n");
      }
      else
      {
        OutputDebugStringA("Failed to create secondary buffer.\n");
      }
    }
  }
}

internal win32_window_dimension Win32GetWindowDimension(HWND Window)
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

internal LRESULT CALLBACK Win32MainWindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_KEYDOWN:
  case WM_KEYUP:
  {
    uint32 VKCode = wParam;
    bool WasDown = ((lParam & (1 << 30)) != 0);
    bool IsDown = ((lParam & (1 << 31)) == 0);

    if (WasDown != IsDown)
    {
      if (VKCode == 'W')
      {
        TRACE("W key %s\n", (IsDown) ? "down" : "up");
      }
      else if (VKCode == 'A')
      {
        TRACE("A key %s\n", (IsDown) ? "down" : "up");
      }
      else if (VKCode == 'S')
      {
        TRACE("S key %s\n", (IsDown) ? "down" : "up");
      }
      else if (VKCode == 'D')
      {
        TRACE("D key %s\n", (IsDown) ? "down" : "up");
      }
      else if (VKCode == VK_UP)
      {
        TRACE("VK_UP key %s\n", (IsDown) ? "down" : "up");
      }
      else if (VKCode == VK_LEFT)
      {
        TRACE("VK_LEFT key %s\n", (IsDown) ? "down" : "up");
      }
      else if (VKCode == VK_DOWN)
      {
        TRACE("VK_DOWN key %s\n", (IsDown) ? "down" : "up");
      }
      else if (VKCode == VK_RIGHT)
      {
        TRACE("VK_RIGHT key %s\n", (IsDown) ? "down" : "up");
      }
      else if (VKCode == VK_ESCAPE)
      {
        Running = false;
      }
      else if (VKCode == VK_SPACE)
      {
        // if (IsDown)
        // {
        //   Pause = !Pause;
        // }
      }
    }
  }

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
  Win32LoadXInput();
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
      Win32InitDSound(hwnd, 48000, 48000 * sizeof(int16) * 2);
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

        for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex)
        {
          XINPUT_STATE ControllerState;
          if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
          {
            XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
            bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
            bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
            bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool RightShoulder = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool A = (Pad->wButtons & XINPUT_GAMEPAD_A);
            bool B = (Pad->wButtons & XINPUT_GAMEPAD_B);
            bool X = (Pad->wButtons & XINPUT_GAMEPAD_X);
            bool Y = (Pad->wButtons & XINPUT_GAMEPAD_Y);

            float StickX = Pad->sThumbLX / 32768.0f;
            float StickY = Pad->sThumbLY / 32768.0f;

            if (A)
            {
              YOffset += 2;
              XINPUT_VIBRATION Vibration = {};
              Vibration.wLeftMotorSpeed = 65535;
              Vibration.wRightMotorSpeed = 65535;
              XInputSetState(ControllerIndex, &Vibration);
              TRACE("A\n");
            }
            TRACE("PAD :%d\n", Pad->bLeftTrigger);
          }
        }

        RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);
        HDC DeviceContext = GetDC(hwnd);
        win32_window_dimension Dimension = Win32GetWindowDimension(hwnd);
        Win32DisplayBufferInWindow(DeviceContext, &GlobalBackBuffer, Dimension.Width, Dimension.Height);
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
