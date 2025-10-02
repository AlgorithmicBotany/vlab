/**************************************************************************

  File:		event.h
  Created:	11-Dec-97


  Declaration of class Event


**************************************************************************/


#ifndef __EVENT_H__
#define __EVENT_H__


class Event
{
public:
	Event(bool ManualReset, bool InitialState);
	~Event();
private:
	HANDLE _hEvent;
};


#endif

