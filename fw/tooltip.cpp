#include <cassert>
#include <string>

#include <windows.h>
#include <commctrl.h>

#include "warningset.h"

#include "mdimenus.h"
#include "exception.h"
#include "menu.h"
#include "window.h"
#include "app.h"
#include "tooltip.h"
#include "winmaker.h"


ToolTip::ToolTip(HWND hOwner, HINSTANCE hInst)
{
	WinMaker maker(TOOLTIPS_CLASS, hInst);
	maker.MakePopup(hOwner);
	_hTT = maker.Create();
}


void ToolTip::Add(Window w, WORD textId, HINSTANCE hInst)
{
	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.hwnd = w.Hwnd();
	ti.uId = (UINT) w.Hwnd();
	ti.hinst = hInst;
	ti.lpszText = (LPTSTR) textId;
	SendMessage(_hTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
}

void ToolTip::Add(const Window& w, int id, const RECT& r)
{
	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = w.Hwnd();
	ti.uId = id;
	ti.rect = r;
	ti.hinst = 0;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage(_hTT, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));
}

void ToolTip::Clear()
{
	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	LRESULT res = SendMessage(_hTT, TTM_ENUMTOOLS, 0, reinterpret_cast<LPARAM>(&ti));
	while (res != FALSE)
	{
		SendMessage(_hTT, TTM_DELTOOL, 0, reinterpret_cast<LPARAM>(&ti));
		res = SendMessage(_hTT, TTM_ENUMTOOLS, 0, reinterpret_cast<LPARAM>(&ti));
	}
}
