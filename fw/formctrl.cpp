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
#include "formctrl.h"
#include "mdimenus.h"
#include "app.h"
#include "fwtmplts.h"


FormCtrl::FormCtrl(HWND hwnd) : Ctrl(hwnd, 0)
{}


FormCtrl::~FormCtrl()
{}


void FormCtrl::SetFocus(HWND, HWND)
{
	App::SetModeless(Hwnd());
}

void FormCtrl::KillFocus(HWND, HWND)
{
	App::ClearModeless();
}


BOOL CALLBACK FormCtrl::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	FormCtrl* pCtrl = GetWinLong<FormCtrl*>(hDlg);
	switch (msg)
	{
	case WM_INITDIALOG :
		{
			try
			{
				Creator* pCreator = reinterpret_cast<Creator*>(lParam);
				pCtrl = pCreator->Create(hDlg);
			}
			catch (Exception e)
			{
				::MessageBox(hDlg, e.Msg(), "Error creating dialog box", MB_ICONSTOP);
				EndDialog(hDlg, 0);
			}
		}
		return true;
	case WM_DESTROY :
		Ctrl::OnDestroy(hDlg);
		return true;
	case WM_HSCROLL :
		return pCtrl->HScroll(static_cast<HScrollCode>(LOWORD(wParam)), static_cast<int>(HIWORD(wParam)), reinterpret_cast<HWND>(lParam));
	case WM_COMMAND :
		return pCtrl->Command(static_cast<int>(LOWORD(wParam)), Window(reinterpret_cast<HWND>(lParam)), static_cast<UINT>(HIWORD(wParam)));
	case WM_SIZE :
		return pCtrl->Size(SizeState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	case WM_TIMER :
		return pCtrl->Timer(static_cast<UINT>(wParam));
	case WM_CTLCOLOREDIT:
		{
			HBRUSH hBr = pCtrl->CtlColorEdit(reinterpret_cast<HDC>(wParam), reinterpret_cast<HWND>(lParam));
			if (0 == hBr)
				return false;
			else
				return BOOL(hBr);
		}
	case WM_MEASUREITEM:
		return pCtrl->MeasureItem(OwnerDraw::Measure(reinterpret_cast<MEASUREITEMSTRUCT*>(lParam)));
	case WM_DRAWITEM:
		return pCtrl->DrawItem(OwnerDraw::Draw(reinterpret_cast<const DRAWITEMSTRUCT*>(lParam)));
	}
	return false;
}
