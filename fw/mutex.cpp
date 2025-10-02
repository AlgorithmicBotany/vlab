/**************************************************************************

  File:		mutex.cpp
  Created:	10-Nov-00


  Implementation of class Mutex


**************************************************************************/

#include <strstream>
#include <string>
#include <cassert>
//#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#include "warningset.h"

#include "mutex.h"
#include "exception.h"
#include "libstrng.h"



Mutex::Mutex(const std::string& name)
{
	_hMutex = CreateMutex(0, FALSE, name.c_str());
	if (0 == _hMutex)
		throw Exception(0, FWStr::CreatingSynchObject);
	assert(ERROR_ALREADY_EXISTS != GetLastError());
}



Mutex::~Mutex()
{
	if (0 != _hMutex)
		CloseHandle(_hMutex);
}


MutexLock::MutexLock(Mutex& mutex, DWORD timeout, bool throwiffailed) : _mutex(mutex)
{
	DWORD res = mutex._Lock(timeout);
	if (WAIT_TIMEOUT == res) 
	{
		if (throwiffailed)
			throw Exception(0, FWStr::Timeout);
		else
			_locked = false;
	}
	else
		_locked = true;
}

