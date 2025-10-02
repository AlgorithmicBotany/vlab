/**************************************************************************

  File:		event.cpp
  Created:	11-Dec-97


  Implementation of class Event


**************************************************************************/


#include <windows.h>

#include "warningset.h"

#include "event.h"
#include "exception.h"

#include "libstrng.h"


Event::Event(bool manreset, bool initstate)
{
	_hEvent = CreateEvent(
		0, 
		manreset,
		initstate,
		0);

	if (0 == _hEvent)
		throw Exception(0, FWStr::CreatingSynchObject);
}

Event::~Event()
{
	CloseHandle(_hEvent);
}
