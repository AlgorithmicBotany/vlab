#include <cassert>

#include <string>

#include <windows.h>
#include <windowsx.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "window.h"
#include "sizestate.h"
#include "keystate.h"
#include "ownerdraw.h"
#include "ctrl.h"
#include "winmaker.h"
#include "genctrl.h"
#include "wndclass.h"


bool GenCtrl::OnCreate(HWND hWnd, const CREATESTRUCT* pCS)

{
	AbstractMaker* pACM = reinterpret_cast<AbstractMaker*>(pCS->lpCreateParams);
	try
	{
		Ctrl* pCtrl = pACM->Create(hWnd, pCS);
		return (0 != pCtrl);
	}
	catch (Exception e)
	{
		MessageBeep(0xFFFFFFFF);
		MessageBox(0, e.Msg(), "Stop", MB_ICONSTOP | MB_OK);
		return false;
	}
	catch (...)
	{
		MessageBox(0, "Unknown error", "Halt", MB_ICONSTOP | MB_OK);
		return false;
	}
}



LRESULT CALLBACK GenCtrl::Proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Ctrl* pCtrl = GetWinLong<Ctrl*>(hWnd);
	switch (msg)
	{
	case WM_ACTIVATE :
		if (pCtrl && pCtrl->Activate(static_cast<Ctrl::ActiveState>(LOWORD(wParam)), reinterpret_cast<HWND>(lParam)))
			return 0;
		break;
	case WM_CAPTURECHANGED :
		if (pCtrl && pCtrl->CaptureChanged())
			return 0;
		break;
	case WM_CHAR :
		if (pCtrl && pCtrl->Char(static_cast<TCHAR>(wParam)))
			return 0;
		break;
	case WM_CLOSE :
		if (pCtrl && !pCtrl->Close())
			return 0;
		break;
	case WM_COMMAND :
		if (pCtrl && pCtrl->Command(static_cast<int>(LOWORD(wParam)), Window(reinterpret_cast<HWND>(lParam)), static_cast<UINT>(HIWORD(wParam))))
			return 0;
		break;
	case WM_CONTEXTMENU :
		if (pCtrl)
		{
			pCtrl->ContextMenu(reinterpret_cast<HWND>(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		}
		break;
	case WM_CREATE :
		if (OnCreate(hWnd, reinterpret_cast<const CREATESTRUCT*>(lParam)))
			return 0;
		else
			return -1;
		break;
	case WM_CTLCOLOREDIT:
		if (pCtrl)
		{
			HBRUSH hBr = pCtrl->CtlColorEdit(reinterpret_cast<HDC>(wParam), reinterpret_cast<HWND>(lParam));
			if (0 != hBr)
				return reinterpret_cast<LRESULT>(hBr);
		}
		break;
	case WM_DESTROY :
		Ctrl::OnDestroy(hWnd);
		return 0;
	case WM_DRAWITEM :
		if (pCtrl && pCtrl->DrawItem(OwnerDraw::Draw(reinterpret_cast<const DRAWITEMSTRUCT*>(lParam))))
			return TRUE;
		break;
	case WM_ERASEBKGND :
		if (pCtrl && pCtrl->EraseBackground(reinterpret_cast<HDC>(wParam)))
			return TRUE;
		break;
	case WM_GETDLGCODE :
		if (pCtrl)
			return pCtrl->GetDlgCode();
		break;
	case WM_HSCROLL :
		if (pCtrl && pCtrl->HScroll(static_cast<Ctrl::HScrollCode>(LOWORD(wParam)), static_cast<int>(HIWORD(wParam)), reinterpret_cast<HWND>(lParam)))
			return 0;
		break;
	case WM_INITMENU :
		if (pCtrl && pCtrl->InitMenu(MenuManipulator(reinterpret_cast<HMENU>(wParam))))
			return 0;
		break;
	case WM_KEYDOWN :
		if (pCtrl && pCtrl->KeyDown(static_cast<UINT>(wParam)))
			return 0;
		break;
	case WM_KILLFOCUS :
		if (pCtrl && pCtrl->KillFocus(reinterpret_cast<HWND>(wParam)))
			return 0;
		break;
	case WM_LBUTTONDBLCLK :
		if (pCtrl && pCtrl->LBDblClick(KeyState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			return 0;
		break;
	case WM_LBUTTONDOWN :
		if (pCtrl && pCtrl->LButtonDown(KeyState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			return 0;
		break;
	case WM_LBUTTONUP :
		if (pCtrl && pCtrl->LButtonUp(KeyState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			return 0;
		break;
	case WM_MBUTTONDOWN :
		if (pCtrl && pCtrl->MButtonDown(KeyState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			return 0;
		break;
	case WM_MBUTTONUP :
		if (pCtrl && pCtrl->MButtonUp(KeyState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			return 0;
		break;
	case WM_MEASUREITEM :
		if (pCtrl && pCtrl->MeasureItem(OwnerDraw::Measure(reinterpret_cast<MEASUREITEMSTRUCT*>(lParam))))
			return TRUE;
		break;
	case WM_MOUSEMOVE :
		if (pCtrl && pCtrl->MouseMove(KeyState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			return 0;
		break;
	case WM_MOUSEWHEEL :
		if (pCtrl && pCtrl->MouseWheel(static_cast<short int>(HIWORD(wParam))))
			return 0;
		break;
	case WM_MOVE :
		if (pCtrl && pCtrl->OnMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			return 0;
		break;
	case WM_MOVING :
		if (pCtrl && pCtrl->Moving(reinterpret_cast<RECT*>(lParam)))
			return TRUE;
		break;
	case WM_NCHITTEST :
		{
			LRESULT res;
			if (pCtrl && pCtrl->NCHitTest(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), res))
				return res;
		}
		break;
	case WM_NOTIFY :
		if (pCtrl)
			return pCtrl->Notify(static_cast<int>(wParam), reinterpret_cast<const NMHDR*>(lParam));
		break;
	case WM_PAINT :
		if (pCtrl && pCtrl->Paint())
			return 0;
		break;
	case WM_RBUTTONDOWN :
		if (pCtrl && pCtrl->RButtonDown(KeyState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			return 0;
		break;
	case WM_SETFOCUS :
		if (pCtrl && pCtrl->SetFocus(reinterpret_cast<HWND>(wParam)))
			return 0;
		break;
	case WM_SIZE :
		if (pCtrl && pCtrl->OnSize(SizeState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			return 0;
		break;
	case WM_TIMER :
		if (pCtrl && pCtrl->Timer(static_cast<UINT>(wParam)))
			return 0;
		break;
	case WM_VSCROLL :
		if (pCtrl && pCtrl->VScroll(static_cast<Ctrl::VScrollCode>(LOWORD(wParam)), static_cast<int>(HIWORD(wParam)), reinterpret_cast<HWND>(lParam)))
			return 0;
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}



void GenCtrl::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, ClassName, Proc);
	wc.Register();
}

void GenCtrl::Unregister(HINSTANCE hInst)
{
	::UnregisterClass(ClassName, hInst);
}

