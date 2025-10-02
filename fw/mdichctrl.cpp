/**************************************************************************

  File:		mdichctrl.cpp
  Created:	30-Jan-98


  Implementation of class MDIChildCtrl


**************************************************************************/


#include <cassert>
#include <string>

#include <windows.h>
#include <windowsx.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "window.h"
#include "keystate.h"
#include "sizestate.h"
#include "ownerdraw.h"
#include "ctrl.h"
#include "mdichctrl.h"
#include "mdimenus.h"

MDIChildCtrl::MDIChildCtrl(HWND hwnd, const CREATESTRUCT* pCS, int docTypeId) : 
Ctrl(hwnd, pCS), 
_DocTypeID(docTypeId),
_active(false)
{}


MDIChildCtrl::~MDIChildCtrl()
{}


bool MDIChildCtrl::MDIActivate(bool activate, HWND hActivate)
{
	if (activate)
	{
		SetDocumentMenu();
		_active = true;
	}
	else
	{
		if (0 == hActivate)
			SetDefaultMenu();
		_active = false;
	}
	return true;
}


void MDIChildCtrl::SetDocumentMenu()
{
	FORWARD_WM_MDISETMENU
		(
		GetParent(), true, 
		DocumentMenu(),
		DocumentWindowMenu(),
		SendMessage
		);
	DrawMenuBar(::GetParent(GetParent()));
}


void MDIChildCtrl::SetDefaultMenu()
{
	FORWARD_WM_MDISETMENU
		(
		GetParent(), true,
		MDIMenus::pGlobalMenus->GetDefaultMenu(), 
		MDIMenus::pGlobalMenus->GetDefaultWindowMenu(),
		SendMessage
		);
	DrawMenuBar(::GetParent(GetParent()));
}


void MDIChildCtrl::Activate()
{
	assert(!_active);
	SendMessage(GetParent(), WM_MDIACTIVATE, (WPARAM) Hwnd(), 0);	
}

HMENU MDIChildCtrl::DocumentMenu() const
{
	return MDIMenus::pGlobalMenus->GetMenu(_DocTypeID);
}

HMENU MDIChildCtrl::DocumentWindowMenu() const
{
	return MDIMenus::pGlobalMenus->GetWindowMenu(_DocTypeID);
}

