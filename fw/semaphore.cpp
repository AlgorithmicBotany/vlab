/**************************************************************************

  File:		semaphore.cpp
  Created:	04-Feb-98


  Implementation of class Semaphore


**************************************************************************/


#include <assert.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#include "warningset.h"

#include "semaphore.h"
#include "exception.h"
#include "libstrng.h"

const char* NameFmt = "Mtx%08x";

Semaphore::Semaphore(LONG init, LONG maximum, const TCHAR* name)
{
	_hSemaphore = CreateSemaphore(
		0,
		init,
		maximum,
		name);
}


Semaphore::Semaphore(LONG init, LONG maximum, unsigned int id)
{
	TCHAR name[16];
	_stprintf(name, NameFmt, id);
	_hSemaphore = CreateSemaphore(
		0,
		init,
		maximum,
		name);
}

Semaphore::Semaphore(unsigned int id, AccessRights access)
{
	char name[16];
	sprintf(name, NameFmt, id);
	_hSemaphore = OpenSemaphore
		(
		access,
		FALSE,
		name
		);
	if (0 == _hSemaphore)
		throw Exception(0, FWStr::OpenSemaphore, name);
}

void Semaphore::Open(unsigned int id, AccessRights access)
{
	assert(0 == _hSemaphore);
	char name[16];
	sprintf(name, NameFmt, id);
	_hSemaphore = OpenSemaphore
		(
		access,
		FALSE,
		name
		);
	if (0 == _hSemaphore)
		throw Exception(0, FWStr::OpenSemaphore, name);
}

/*
Semaphore::Semaphore(const TCHAR* name, DWORD access)
{
	_hSemaphore = OpenSemaphore
		(
		access,
		false,
		name
		);
}
*/


MultiUserSemaphore::MultiUserSemaphore(LONG init, LONG maximum)
{
	_init = init;
	_maximum = maximum;
	_RefCount = 0;
	_pSemaphore = 0;
}

MultiUserSemaphore::~MultiUserSemaphore()
{
	assert(0 == _RefCount);
	assert(0 == _pSemaphore);
}


void MultiUserSemaphore::NewUser()
{
	_RefCount++;
	if (1==_RefCount)
	{
		assert(0 == _pSemaphore);
		_pSemaphore = new Semaphore(_init, _maximum, (const TCHAR*)0);
	}
}


void MultiUserSemaphore::Detach()
{
	_RefCount--;
	assert(_RefCount>=0);
	if (0==_RefCount)
	{
		delete _pSemaphore;
		_pSemaphore = 0;
	}
}
