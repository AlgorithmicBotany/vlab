/**************************************************************************

  File:		tabctrl.cpp
  Created:	02-Feb-98


  Implementation of class TabCtrl


**************************************************************************/


#include <cassert>

#include <string>

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "window.h"
#include "tabctrl.h"
#include "mdimenus.h"
#include "app.h"
#include "lstring.h"
#include "winmaker.h"
#include "resstrng.h"



TabCtrl::TabCtrl(const Window* pParent, const UINT* arr, int items, int childid)
{
	WinMaker wm(WC_TABCONTROL, App::GetInstance());
	wm.MakeChild(childid, pParent->Hwnd());
	wm.MakeVisible();
	SetHwnd(wm.Create());

	for (int i=0; i<items; i++)
	{
		ResString label(32, arr[i]);
		TC_ITEM tci;
		{
			tci.mask    = TCIF_TEXT | TCIF_IMAGE;
			tci.pszText = &(label[0]);
			tci.iImage  = i;
		}
		TabCtrl_InsertItem(Hwnd(), i, &tci);
	}
}

void TabCtrl::SetTabText(const UINT* arr, int items)
{
	for (int i=0; i<items; i++)
	{
		ResString label(32, arr[i]);
		TC_ITEM tci;
		{
			tci.mask    = TCIF_TEXT | TCIF_IMAGE;
			tci.pszText = &(label[0]);
			tci.iImage  = -1;
		}
		TabCtrl_SetItem(Hwnd(), i, &tci);
	}
}


void TabCtrl::SetTabIcons(int items)
{
	for (int i=0; i<items; i++)
	{
		TC_ITEM tci;
		{
			tci.mask    = TCIF_TEXT | TCIF_IMAGE;
			tci.pszText = 0;
			tci.iImage  = i;
		}
		TabCtrl_SetItem(Hwnd(), i, &tci);
	}
}

void TabCtrl::SetTabTextAndIcons(const UINT* arr, int items)
{
	for (int i=0; i<items; i++)
	{
		ResString label(32, arr[i]);
		TC_ITEM tci;
		{
			tci.mask    = TCIF_TEXT | TCIF_IMAGE;
			tci.pszText = &(label[0]);
			tci.iImage  = i;
		}
		TabCtrl_SetItem(Hwnd(), i, &tci);
	}
}

void TabCtrl::SetTabText(int id, const std::string& label)
{
	TC_ITEM tci;
	tci.mask    = TCIF_TEXT | TCIF_IMAGE;
	tci.pszText = const_cast<char*>(&(label[0]));
	tci.iImage  = -1;
	TabCtrl_SetItem(Hwnd(), id, &tci);
}

void TabCtrl::SetTabTextAndIcon(int id, const std::string& label)
{
	TC_ITEM tci;
	tci.mask    = TCIF_TEXT | TCIF_IMAGE;
	tci.pszText = const_cast<char*>(&(label[0]));
	tci.iImage  = id;
	TabCtrl_SetItem(Hwnd(), id, &tci);
}
