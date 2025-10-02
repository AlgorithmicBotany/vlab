#include <assert.h>
#include <string>

#include <windows.h>
#include <commctrl.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "window.h"
#include "spinctrl.h"


void SpinCtrl::SetRange(int min, int max)
{
	SendMessage(Hwnd(), UDM_SETRANGE, 0, (LPARAM)MAKELONG((short) min, (short) max));
}


void SpinCtrl::SetPos(int pos)
{
	SendMessage(Hwnd(), UDM_SETPOS, 0, (LPARAM)MAKELONG((short) pos, 0));
}
