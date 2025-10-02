/**************************************************************************

  File:		mdiclient.cpp
  Created:	25-Nov-97


  Implementation of class MDIClient


**************************************************************************/


#include <cassert>
#include <string>

#include <windows.h>
#include <commctrl.h>

#include "warningset.h"

#include "mdimenus.h"
#include "exception.h"
#include "menu.h"
#include "window.h"
#include "mdiclient.h"
#include "winmaker.h"

static const int FirstMDIChild = 101;


MDIClient::MDIClient(HWND hParent, HINSTANCE hInst)
{
	assert(0 != MDIMenus::pGlobalMenus);
	CLIENTCREATESTRUCT ccs;
	{
		ccs.hWindowMenu = MDIMenus::pGlobalMenus->GetDefaultWindowMenu();
		ccs.idFirstChild= FirstMDIChild;
	}

	WinMaker maker(__TEXT("MDICLIENT"), hInst);
	maker.MakeChild(2, hParent);
	maker.MakeClipChildren();
	maker.MakeVisible();
	maker.lpData(&ccs);
	SetHwnd(maker.Create());
}
