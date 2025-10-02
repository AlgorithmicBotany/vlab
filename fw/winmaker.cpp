#include <cassert>

#include <string>

#include <windows.h>


#include "warningset.h"

#include "mdimenus.h"
#include "menu.h"
#include "exception.h"
#include "window.h"
#include "app.h"
#include "winmaker.h"
#include "libstrng.h"


WinMaker::WinMaker(const TCHAR* cls, HINSTANCE hInst)
{
	if (0==hInst)
		hInst = App::GetInstance();
	cs.dwExStyle = 0;
	cs.lpszClass = cls;
	cs.lpszName = "";
	cs.style = 0;
	cs.x = cs.y = CW_USEDEFAULT;
	cs.cx = cs.cy = CW_USEDEFAULT;
	cs.hwndParent = 0;
	cs.hMenu = 0;
	cs.hInstance = hInst;
	cs.lpCreateParams = 0;
}

HWND WinMaker::Create() const
{
	HWND hRes = CreateWindowEx
		(
		cs.dwExStyle,
		cs.lpszClass,
		cs.lpszName,
		cs.style,
		cs.x, cs.y,
		cs.cx, cs.cy,
		cs.hwndParent,
		cs.hMenu,
		cs.hInstance,
		cs.lpCreateParams
		);
	if (0==hRes)
	{
		DWORD err = GetLastError();
		if (err != ERROR_SUCCESS)
			throw Exception(FWStr::CreateWindowClss, cs.lpszClass, err);
	}
	return hRes;
}
