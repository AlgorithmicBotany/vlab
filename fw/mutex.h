#ifndef __MUTEX_H__
#define __MUTEX_H__


class Mutex
{
friend class MutexLock;
public:
	Mutex(const std::string& name);
	//Mutex(unsigned int);
	~Mutex();
	HANDLE Release()
	{
		HANDLE hRes = _hMutex;
		_hMutex = 0;
		return hRes;
	}
private:
	DWORD _Lock(DWORD timeout)
	{ return WaitForSingleObject(_hMutex, timeout); }
	void _Unlock()
	{ ReleaseMutex(_hMutex); }
	HANDLE _hMutex;
};

class MutexLock
{
public:
	MutexLock(Mutex& mutex, DWORD timeout = INFINITE, bool /*throwiffailed*/ = true);
	~MutexLock()
	{ _mutex._Unlock(); }
	bool Locked() const
	{ return _locked; }
private:
	Mutex& _mutex;
	bool _locked;
};


#endif
