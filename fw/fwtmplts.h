#ifndef __FWTMPLTS_H__
#define __FWTMPLTS_H__


template <typename T>
bool OnCreate(HWND hwnd, const CREATESTRUCT* pCS, T* = 0)
{
	try
	{
		new T(hwnd, pCS);
	}
	catch (Exception e)
	{
		::MessageBox(hwnd, e.Msg(), "Error creating window", MB_ICONSTOP);
		return false;
	}
	return true;
}


template <typename T>
class Wnd
{
public:
	static LRESULT CALLBACK Proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		T* pCtrl = GetWinLong<T*>(hWnd);
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
			if (OnCreate<T>(hWnd, reinterpret_cast<const CREATESTRUCT*>(lParam)))
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
			T::OnDestroy(hWnd);
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
			if (pCtrl && pCtrl->MouseWheel((short int)HIWORD(wParam)))
				return 0;
			break;
		case WM_MOVE :
			if (pCtrl && pCtrl->OnMove(LOWORD(lParam), HIWORD(lParam)))
				return 0;
			break;
		case WM_NOTIFY :
			return (pCtrl && pCtrl->Notify(static_cast<int>(wParam), reinterpret_cast<const NMHDR*>(lParam)));
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
};


template <typename T>
class MDIWnd
{
public:
	static LRESULT CALLBACK Proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		T* pCtrl = GetWinLong<T*>(hWnd);
		switch (msg)
		{
		case WM_CLOSE :
			if (pCtrl && !pCtrl->Close())
				return 0;
			break;
		case WM_COMMAND :
			if (pCtrl)
				pCtrl->Command(GET_WM_COMMAND_ID(wParam, lParam), Window(reinterpret_cast<HWND>(lParam)), static_cast<UINT>(HIWORD(wParam)));
			break;
		case WM_CREATE :
			if (OnCreate<T>(hWnd, reinterpret_cast<const CREATESTRUCT*>(lParam)))
				return 0;
			else
				return -1;
			break;
		case WM_DESTROY :
			T::OnDestroy(hWnd);
			return 0;
		case WM_DROPFILES :
			if (pCtrl && pCtrl->DropFiles(reinterpret_cast<HDROP>(wParam)))
				return 0;
			break;
		case WM_EXITMENULOOP :
			if (pCtrl && pCtrl->ExitMenuLoop())
				return 0;
			break;
		case WM_INITMENU :
			if (pCtrl && pCtrl->InitMenu(MenuManipulator(reinterpret_cast<HMENU>(wParam))))
				return 0;
			break;
		case WM_KILLFOCUS :
			if (pCtrl && pCtrl->KillFocus(reinterpret_cast<HWND>(wParam)))
				return 0;
			break;
		case WM_MENUSELECT :
			if (pCtrl && pCtrl->MenuSelect(LOWORD(wParam), HIWORD(wParam)))
				return 0;
			break;
		case WM_MOVE :
			if (pCtrl && pCtrl->OnMove(LOWORD(lParam), HIWORD(lParam)))
				return 0;
			break;
		case WM_NOTIFY :
			return (pCtrl && pCtrl->Notify(static_cast<int>(wParam), reinterpret_cast<const NMHDR*>(lParam)));
		case WM_QUERYENDSESSION :
			if (pCtrl && pCtrl->QueryEndSession())
				return 0;
			break;
		case WM_SETFOCUS :
			if (pCtrl)
				pCtrl->SetFocus(reinterpret_cast<HWND>(wParam));
			break;
		case WM_SHOWWINDOW :
			if (pCtrl && pCtrl->ShowWindow(wParam == TRUE))
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
		}
		return DefFrameProc(hWnd, pCtrl ? pCtrl->MdiClient()->Hwnd() : 0, msg, wParam, lParam);
	}
};


template <typename T>
class MDIChWnd
{
public:
	static LRESULT CALLBACK Proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		T* pCtrl = GetWinLong<T*>(hWnd);
		switch (msg)
		{
		case WM_CLOSE :
			if (!pCtrl->Close())
				return 0;
			break;
		case WM_COMMAND:
			if (pCtrl->Command(GET_WM_COMMAND_ID(wParam, lParam), Window(reinterpret_cast<HWND>(lParam)), HIWORD(wParam)))
				return 0;
			break;
		case WM_CREATE :
			if (OnCreate<T>(hWnd, reinterpret_cast<const CREATESTRUCT*>(lParam)))
				return 0;
			else
				return -1;
			break;
		case WM_DESTROY :
			T::OnDestroy(hWnd);
			return 0;
		case WM_INITMENU:
			if (pCtrl->InitMenu(MenuManipulator(reinterpret_cast<HMENU>(wParam))))
				return 0;
			break;
		case WM_MDIACTIVATE:
			if (pCtrl->MDIActivate(hWnd == reinterpret_cast<HWND>(lParam), reinterpret_cast<HWND>(lParam)))
				return 0;
			break;
		case WM_MOUSEACTIVATE:
			{
				LRESULT res;
				if (pCtrl->MouseActivate(res))
					return res;
			}
			break;
		case WM_MOVE :
			if (pCtrl)
				pCtrl->OnMove(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_NOTIFY :
			return (pCtrl && pCtrl->Notify(static_cast<int>(wParam), reinterpret_cast<const NMHDR*>(lParam)));
		case WM_QUERYENDSESSION :
			if (pCtrl->QueryEndSession())
				return TRUE;
			else
				return FALSE;
		case WM_SIZE :
			pCtrl->OnSize(SizeState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case WM_TIMER:
			if (pCtrl->Timer(static_cast<UINT>(wParam)))
				return 0;
			break;
		}
		return DefMDIChildProc(hWnd, msg, wParam, lParam);
	}
};




#endif
