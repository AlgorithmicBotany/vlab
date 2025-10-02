#ifndef __NEWPROCESS_H__
#define __NEWPROCESS_H__


class NewProcess
{
public:
	NewProcess(const std::string&, UINT);
	~NewProcess();
	HANDLE ReleaseProcess();
	DWORD ProcessId() const
	{ return _pi.dwProcessId; }
private:
	void Create(const std::string&, UINT);
	PROCESS_INFORMATION _pi;
};


#endif
