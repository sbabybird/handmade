/**
 * @file trace.cpp
 * @author SunMinjie (smj@ieforever.com)
 * @brief https://stackoverflow.com/questions/494653/how-can-i-use-the-trace-macro-in-non-mfc-projects
 * @version 0.1
 * @date 2021-11-27
 * @copyright Copyright (c) 2021
 *
 */
#include "trace.h"
#ifdef _DEBUG
bool _trace(TCHAR *format, ...)
{
  TCHAR buffer[1000];

  va_list argptr;
  va_start(argptr, format);
  wvsprintf(buffer, format, argptr);
  va_end(argptr);

  OutputDebugString(buffer);

  return true;
}
#endif