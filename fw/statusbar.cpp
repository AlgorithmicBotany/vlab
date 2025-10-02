/**************************************************************************

  File:		statusbar.cpp
  Created:	25-Nov-97


  Implementation of class StatusBar


**************************************************************************/


#include <assert.h>

#include <string>

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "window.h"
#include "sizestate.h"
#include "statusbar.h"


StatusBar::StatusBar(HWND hParent) :
Window(CreateStatusWindow(
	   WS_CHILD | WS_VISIBLE,
	   __TEXT(""),
	   hParent,
	   chId))
{
	RECT r; GetRect(r);
	_height = r.bottom;
}


void StatusBar::Size(SizeState sst, int w, int h)
{
	FORWARD_WM_SIZE(Hwnd(), sst.Value(), w, h, SendMessage);
}
