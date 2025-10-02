/**************************************************************************

  File:		thread.cpp
  Created:	11-Dec-97


  Implementation of class Thread


**************************************************************************/


#include <windows.h>

#include "warningset.h"

#include "thread.h"
#include "exception.h"
#include "libstrng.h"

#ifndef FW_STHREAD


Thread::Thread(LPTHREAD_START_ROUTINE pProc, void* pVoid, DWORD flags)
{
	_hThread = CreateThread(
		0,
		0,
		pProc,
		pVoid,
		flags,
		&_id);

	if (0 == _hThread)
		throw Exception(0, FWStr::CreatingThread);
}

Thread::~Thread()
{
	CloseHandle(_hThread);
}


#endif /* FW_STHREAD */
