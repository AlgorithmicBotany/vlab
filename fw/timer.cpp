#include <assert.h>
#include <string>

#include <windows.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "window.h"
#include "timer.h"

FW::Timer::Timer(Window* pWnd, UINT id, UINT timeout, bool start) :
_id(id), _timeout(timeout)
{
	_hWnd = pWnd->Hwnd();
	_active = start;
	if (_active)
		Start();
}

void FW::Timer::Start()
{
	if (!_active)
	{
		::SetTimer(_hWnd, _id, _timeout, 0);
		_active = true;
	}
}

void FW::Timer::Kill()
{
	if (_active)
	{
		::KillTimer(_hWnd, _id);
		_active = false;
	}
}
