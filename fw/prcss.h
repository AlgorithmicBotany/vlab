#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "window.h"

template <typename HandleType, HandleType NullValue = 0>
class Handle
{
public:
	explicit Handle(HandleType h = NullValue) : _h(h) {}
	~Handle ()
	{
		if (!IsNull())
			Close();
	}
	bool IsNull() const
	{ return _h == NullValue; }
	void Reset(HandleType h)
	{
		assert(IsNull());
		_h = h;
	}
	void Close()
	{
		assert(!IsNull());
		CloseHandle(_h);
		_h = NullValue;
	}
protected:
	HandleType _h;
};


namespace Process
{

class StartupInfo
{
public:
	StartupInfo()
	{
		_si.cb = sizeof(STARTUPINFO);
		_si.lpReserved = 0;
		_si.lpDesktop = 0;
		_si.lpTitle = 0;
		_si.dwX = _si.dwY = _si.dwXSize = _si.dwYSize = 0;
		_si.dwXCountChars = _si.dwYCountChars = 0;
		_si.dwFillAttribute = 0;
		_si.dwFlags = 0;
		_si.wShowWindow = 0;
		_si.cbReserved2 = 0;
		_si.lpReserved2 = 0;
		_si.hStdInput = _si.hStdOutput = _si.hStdError = 0;
	}
	STARTUPINFO* Ptr()
	{ return &_si; }
	void Show(Window::sw sw)
	{
		_si.wShowWindow = static_cast<WORD>(sw);
		SetFlag(UseShowWindow);
	}
	enum sf
	{
		UsePosition = STARTF_USEPOSITION,
		UseShowWindow = STARTF_USESHOWWINDOW,
		UseSize = STARTF_USESIZE,
		UseStdHandles = STARTF_USESTDHANDLES
	};
	void SetFlag(sf flag)
	{ _si.dwFlags |= flag; }
private:
	STARTUPINFO _si;
};

class Information
{
public:
	Information()
	{
		_pi.dwProcessId = 0;
		_pi.dwThreadId = 0;
		_pi.hProcess = 0;
		_pi.hThread = 0;
	}
	~Information()
	{
		if (0 != _pi.hThread)
			CloseHandle(_pi.hThread);
		if (0 != _pi.hProcess)
			CloseHandle(_pi.hProcess);
	}
	DWORD ProcessId() const
	{ return _pi.dwProcessId; }
	HANDLE ReleaseProcess()
	{
		HANDLE hRes = _pi.hProcess;
		_pi.hProcess = 0;
		return hRes;
	}
	PROCESS_INFORMATION* Ptr()
	{ return &_pi; }
private:
	PROCESS_INFORMATION _pi;
};



class Maker
{
public:
	Maker(const std::string& cmnd, const std::string& params) : _cmndln(cmnd)
	{ 
		if (!_cmndln.empty())
			_cmndln.append(1, ' ');
		_cmndln.append(params);
	}
	bool Create(const std::string& dir)
	{
		Process::StartupInfo si;
		BOOL res = CreateProcess
			(
			0, 
			&(_cmndln[0]),
			0, 0,
			FALSE, 
			0, 0,
			dir.c_str(),
			si.Ptr(),
			_pi.Ptr()
			);
		return res != 0;
	}
	const char* CommandLine() const
	{ return _cmndln.c_str(); }
	DWORD ProcessId() const
	{ return _pi.ProcessId(); }
	HANDLE ReleaseProcess()
	{ return _pi.ReleaseProcess(); }
private:
	std::string _cmndln;
	Process::Information _pi;
};


class Process : public Handle<HANDLE>
{
public:
	Process(HANDLE h = 0) : Handle<HANDLE>(h) {}
	bool Wait(DWORD ms) const
	{ return WAIT_OBJECT_0 == WaitForSingleObject(_h, ms); }
	void Terminate() const
	{ TerminateProcess(_h, 0); }
	bool IsRunning() const
	{ 
		if (IsNull())
			return false;
		else
			return !Wait(0); 
	}
};

}



#endif
