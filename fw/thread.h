/**************************************************************************

  File:		thread.h
  Created:	11-Dec-97


  Declaration of class Thread


**************************************************************************/


#ifndef __THREAD_H__
#define __THREAD_H__


#ifndef FW_STHREAD

class Thread
{
public:
	Thread(LPTHREAD_START_ROUTINE, void*, DWORD);
	~Thread();
	DWORD Resume()
	{ return ResumeThread(_hThread); }
	bool Wait(DWORD timeout = INFINITE)
	{ return WAIT_OBJECT_0 == WaitForSingleObject(_hThread, timeout); }
private:
	HANDLE _hThread;
	DWORD  _id;
};


#endif

#endif
