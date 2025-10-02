/**************************************************************************

  File:		exception.cpp
  Created:	24-Nov-97


  Implementation of classes Exception


**************************************************************************/


#include <cassert>
#include <cstdio>

#include <string>

#include <windows.h>
#include <tchar.h>

#include "warningset.h"

#include "mdimenus.h"
#include "exception.h"
#include "menu.h"
#include "window.h"
#include "app.h"

#include "libstrng.h"

char Exception::_msg[] = "";

Exception::Exception(const TCHAR* msg, ...) : _errval(GetLastError())
{
	va_list args;
	va_start(args, msg);
	_vsntprintf(_msg, sizeof(TCHAR)*ExceptionMessageLength, msg, args);
	va_end(args);
}


Exception::Exception(UINT id, ...) : _errval(GetLastError())
{
	static TCHAR bf[ExceptionMessageLength];
	if (0==LoadString(App::GetInstance(), id, bf, ExceptionMessageLength))
	{
		strncpy(bf, FWStr::GetLibString(id), ExceptionMessageLength);
		bf[ExceptionMessageLength-1];
	}
	va_list args;
	va_start(args, id);
	_vsntprintf(_msg, sizeof(TCHAR)*ExceptionMessageLength, bf, args);
	va_end(args);
}


Exception::Exception(HINSTANCE hInst, UINT id, ...) : _errval(GetLastError())
{
	static TCHAR bf[ExceptionMessageLength];
	if (0 != hInst)
		LoadString(hInst, id, bf, ExceptionMessageLength);
	else
	{
		strncpy(bf, FWStr::GetLibString(id), ExceptionMessageLength);
		bf[ExceptionMessageLength-1];
	}
	va_list args;
	va_start(args, id);
	_vsntprintf(_msg, sizeof(TCHAR)*ExceptionMessageLength, bf, args);
	va_end(args);
}


void Exception::AddMsg(HINSTANCE hInst, UINT id, ...)
{
	static TCHAR bf[ExceptionMessageLength];
	LoadString(hInst, id, bf, ExceptionMessageLength);
	static TCHAR bf2[ExceptionMessageLength];
	va_list args;
	va_start(args, id);
	_vsntprintf(bf2, sizeof(TCHAR)*ExceptionMessageLength, bf, args);
	va_end(args);
	int maxappend = ExceptionMessageLength - _tcslen(_msg) - 1;
	if (maxappend>0)
	{
		_tcsncat(_msg, __TEXT("\n"), maxappend);
		maxappend--;
		if (maxappend>0)
			_tcsncat(_msg, bf2, maxappend);
	}
}


void Exception::AddMsg(UINT id, ...)
{
	static TCHAR bf[ExceptionMessageLength];
	LoadString(App::GetInstance(), id, bf, ExceptionMessageLength);
	static TCHAR bf2[ExceptionMessageLength];
	va_list args;
	va_start(args, id);
	_vsntprintf(bf2, sizeof(TCHAR)*ExceptionMessageLength, bf, args);
	va_end(args);
	int maxappend = ExceptionMessageLength - _tcslen(_msg) - 1;
	if (maxappend>0)
	{
		_tcsncat(_msg, __TEXT("\n"), maxappend);
		maxappend--;
		if (maxappend>0)
			_tcsncat(_msg, bf2, maxappend);
	}
}

