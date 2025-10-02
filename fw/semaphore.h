/**************************************************************************

  File:		semaphore.h
  Created:	04-Feb-98


  Declaration of class Semaphore


**************************************************************************/


#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__


class Semaphore
{
public:
	enum AccessRights
	{
		AllAccess = SEMAPHORE_ALL_ACCESS,
		ModifyState = SEMAPHORE_MODIFY_STATE
	};
	Semaphore(LONG, LONG, const TCHAR*);
	Semaphore(LONG, LONG, unsigned int);
	Semaphore(unsigned int, AccessRights);
	Semaphore() { _hSemaphore = 0; }
	//Semaphore(const TCHAR*, DWORD access = SEMAPHORE_ALL_ACCESS);
	~Semaphore()
	{
		if (0 != _hSemaphore)
			CloseHandle(_hSemaphore);
	}
	void Open(unsigned int, AccessRights);
	void Release(LONG n = 1, LONG* pPrev = 0)
	{
		assert(0 != _hSemaphore);
		ReleaseSemaphore(_hSemaphore, n, pPrev);
	}

	bool Wait(DWORD timeout = INFINITE)
	{
		assert(0 != _hSemaphore);
		return (WAIT_OBJECT_0 == WaitForSingleObject(_hSemaphore, timeout)); 
	}
private:
	Semaphore(const Semaphore&);
	HANDLE _hSemaphore;
};


class MultiUserSemaphore
{
public:
	MultiUserSemaphore(LONG, LONG);
	~MultiUserSemaphore();
	void NewUser();
	void Detach();

	Semaphore* operator->()
	{ 
		assert(0 != _pSemaphore);
		return _pSemaphore; 
	}
private:
	LONG _init;
	LONG _maximum;
	int _RefCount;
	Semaphore* _pSemaphore;
};



#endif
