/**************************************************************************

  File:		mdictrl.cpp
  Created:	24-Nov-97


  Implementation of class MDICtrl


**************************************************************************/


#include <cassert>
#include <string>


#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>

#include "warningset.h"

#include "mdimenus.h"
#include "exception.h"
#include "menu.h"
#include "window.h"
#include "sizestate.h"
#include "statusbar.h"
#include "mdiclient.h"
#include "keystate.h"
#include "ownerdraw.h"
#include "ctrl.h"
#include "appctrl.h"
#include "mdictrl.h"
#include "app.h"
#include "childenum.h"




MDICtrl::MDICtrl(HWND hwnd, const CREATESTRUCT* pCS) : 
AppCtrl<StatusBarOn>(hwnd, pCS),
_MDIClient(hwnd, pCS->hInstance)
{
	_hToolbar = 0;
}


MDICtrl::~MDICtrl()
{}


bool MDICtrl::Size(SizeState sst, int w, int h)
{
	AppCtrl<StatusBarOn>::Size(sst, w, h);

	int height = WorkAreaHeight();
	
	int toolbarh = 0;
	
	if (0 != _hToolbar)
	{
		FORWARD_WM_SIZE(_hToolbar, sst.Value(), w, h, SendMessage);
		Window tb(_hToolbar);
		RECT r;
		tb.GetRect(r);
		height -= r.bottom;
		toolbarh = r.bottom;
	}
	
	_MDIClient.MoveWindow(0, toolbarh, w, height);

	return true;
}


bool MDICtrl::CloseAll()
{
	EnumChildWindows(_MDIClient.Hwnd(), MDICtrl::_CloseEnumProc, 0);
	if (0 != GetWindow(_MDIClient.Hwnd(), GW_CHILD))
		return false;
	else
		return true;
}


void MDICtrl::_Exit()
{
	if (CloseAll())
		FORWARD_WM_CLOSE(Hwnd(), SendMessage);
}


BOOL CALLBACK MDICtrl::_CloseEnumProc(HWND hwnd, LPARAM)
{
	if (IsIconic(hwnd))
		FORWARD_WM_MDIRESTORE(::GetParent(hwnd), hwnd, SendMessage);

	if (!(FORWARD_WM_QUERYENDSESSION(hwnd, SendMessage)))
		return false;

	FORWARD_WM_MDIDESTROY(::GetParent(hwnd), hwnd, SendMessage);

	return true;
}


HWND MDICtrl::_NewMDIDocument(const std::string& wc, const std::string& title, const void* lParam)
{
	MDICREATESTRUCT mcs;
	{
		mcs.szClass	= wc.c_str();
		mcs.szTitle	= title.c_str();
		mcs.hOwner	= App::GetInstance();
		mcs.x		= CW_USEDEFAULT;
		mcs.y		= CW_USEDEFAULT;
		mcs.cx		= CW_USEDEFAULT;
		mcs.cy		= CW_USEDEFAULT;
		mcs.style	= 0;
		mcs.lParam	= reinterpret_cast<LPARAM>(lParam);
	}

	HWND hChild = FORWARD_WM_MDICREATE(_MDIClient.Hwnd(), &mcs, SendMessage);
	return hChild;
}


