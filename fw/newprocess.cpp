#include <string>
#include <cassert>

#include <windows.h>

#include "warningset.h"

#include "newprocess.h"
#include "exception.h"
#include "prcss.h"


NewProcess::NewProcess(const std::string& cmndln, UINT err)
{
	Create(&(cmndln[0]), err);
}


void NewProcess::Create(const std::string& cmndln, UINT err)
{
	Process::StartupInfo si;
	si.Show(Window::swShowDefault);
	std::string cmnd(cmndln);

	BOOL res = CreateProcess
		(
		0, &(cmnd[0]),
		0, 0,
		true, 0,
		0, 0,
		si.Ptr(), &_pi
		);

	if (!res)
	{
		_pi.hProcess = 0;
		_pi.hThread = 0;
		throw Exception(err,cmndln.c_str());
	}
	CloseHandle(_pi.hThread);
	_pi.hThread = 0;
}


NewProcess::~NewProcess()
{
	if (0 != _pi.hProcess)
		CloseHandle(_pi.hProcess);
}


HANDLE NewProcess::ReleaseProcess()
{
	assert(0 != _pi.hProcess);
	HANDLE hResult = _pi.hProcess;
	_pi.hProcess = 0;
	return hResult;
}
