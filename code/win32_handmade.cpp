#include <windows.h>

int CALLBACK
WinMain(_In_  HINSTANCE hInstance,
        _In_  HINSTANCE hPrevInstance,
        _In_  LPSTR lpCmdLine,
        _In_  int nCmdShow)
{
  MessageBox(0, "Hello world!", "hello", MB_OK|MB_ICONINFORMATION);

  return 0;
}
