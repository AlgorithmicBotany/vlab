/**************************************************************************

  File:		dialog.cpp
  Created:	01-Dec-97


  Implementation of class Dialog


**************************************************************************/


#include <cassert>

#include <string>

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "window.h"
#include "dialog.h"
#include "mdimenus.h"
#include "app.h"
#include "editline.h"
#include "combobox.h"
#include "button.h"
#include "lstring.h"
#include "libstrng.h"
#include "fwres.h"
#include "resstrng.h"

class InputFormatException : public Exception
{
public:
	InputFormatException(UINT msg, int id) : Exception(0, msg), _id(id)
	{}
	int Id() const
	{ return _id; }
private:
	int _id;
};

Dialog* Dialog::_pCurDlg = 0;

Dialog::Dialog(WORD id, HINSTANCE hInst) : _id(MAKEINTRESOURCE(id)), _hInst(hInst)
{
	_hDlg = 0;
}

Dialog::Dialog(const char* id, HINSTANCE hInst) : _id(id), _hInst(hInst)
{
	_hDlg = 0;
}

int Dialog::_DoModal(HWND hOwner)
{
	assert(0 == _pCurDlg);
	_pCurDlg = this;
	return static_cast<int>(DialogBoxParam(
		_hInst != 0 ? _hInst : App::GetInstance(),
		_id,
		hOwner,
        reinterpret_cast<DLGPROC>(Dialog::DlgProc),
		reinterpret_cast<LPARAM>(this)));
}



BOOL CALLBACK Dialog::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Dialog* pSelf = reinterpret_cast<Dialog*>(GetWindowLongPtr(hDlg, GWLP_USERDATA));
	switch (msg)
	{
	case WM_INITDIALOG :
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		pSelf = reinterpret_cast<Dialog*>(lParam);
		_pCurDlg = 0;
		return pSelf->InitDialog(hDlg);
	case WM_COMMAND :
		try
		{
			pSelf->Command(hDlg, LOWORD(wParam), Window(reinterpret_cast<HWND>(lParam)), HIWORD(wParam));
		}
		catch (Exception e)
		{
			pSelf->ErrorBox(e);
		}
		return true;
	case WM_MEASUREITEM :
		if (_pCurDlg)
			_pCurDlg->HandleMsg(hDlg, msg, wParam, lParam);
		else
			assert(0);
		break;
	case WM_CTLCOLORBTN :
		return (BOOL)HANDLE_WM_CTLCOLORBTN(hDlg, wParam, lParam, pSelf->CtlColor);
	case WM_CTLCOLORDLG :
		return (BOOL)HANDLE_WM_CTLCOLORDLG(hDlg, wParam, lParam, pSelf->CtlColor);
	case WM_CTLCOLOREDIT :
		return (BOOL)HANDLE_WM_CTLCOLOREDIT(hDlg, wParam, lParam, pSelf->CtlColor);
	case WM_CTLCOLORLISTBOX :
		return (BOOL)HANDLE_WM_CTLCOLORLISTBOX(hDlg, wParam, lParam, pSelf->CtlColor);
	case WM_CTLCOLORMSGBOX :
		return (BOOL)HANDLE_WM_CTLCOLORMSGBOX(hDlg, wParam, lParam, pSelf->CtlColor);
	case WM_CTLCOLORSCROLLBAR :
		return (BOOL)HANDLE_WM_CTLCOLORSCROLLBAR(hDlg, wParam, lParam, pSelf->CtlColor);
	case WM_CTLCOLORSTATIC :
		return (BOOL)HANDLE_WM_CTLCOLORSTATIC(hDlg, wParam, lParam, pSelf->CtlColor);
	}
	return (0 != pSelf) ? pSelf->HandleMsg(hDlg, msg, wParam, lParam) : false;
}


bool Dialog::InitDialog(HWND hDlg)
{
	_hDlg = hDlg;
	try
	{
		UpdateData(false);
		bool toret = DoInit();
		return toret;
	}
	catch (Exception e)
	{
		ErrorBox(e);
		EndDialog(IDCANCEL);
	}
	return true;
}

void Dialog::Command(HWND, int id, Window ctl, UINT notify)
{
	if (!Command(id, ctl, notify))
	{
		switch (id)
		{
		case IDCANCEL :
			EndDialog(IDCANCEL);
			break;
		case IDOK :
			try
			{
				UpdateData(true);
				if (_Check())
					EndDialog(IDOK);
			}
			catch (InputFormatException e)
			{
				ErrorBox(e);
				Window Ctl(GetDlgItem(e.Id()));
				Ctl.GrabFocus();
			}
			break;
		}
	}
}


void Dialog::ScreenToClient(RECT& r)
{
	MapWindowPoints(HWND_DESKTOP, _hDlg, (POINT*) &r, 2);
}


void Dialog::_CheckFailed(UINT msg, int id)
{
	ResString MsgTxt(128, msg);
	MessageBox(_hDlg, MsgTxt.c_str(), FWStr::GetLibString(FWStr::ErrorCaption), MB_ICONERROR);
	if (0 != id)
	{
		Window Ctl(GetDlgItem(id));
		Ctl.GrabFocus();
	}
}


void Dialog::EnableItem(int id, bool enable)
{
	Window Ctl(GetDlgItem(id));
	Ctl.Enable(enable);
}


void Dialog::_InvalidFormat(UINT msg, int id)
{
	throw InputFormatException(msg, id);
}




/*********************************************************************************

  D X  ----  D a t a   e x c h a n g e    f u n c t i o n s

*********************************************************************************/


void Dialog::DX(LongString& str, int id, bool what)
{
	EditLine el(GetDlgItem(id));
	if (what)
		el.GetText(str);
	else
	{
		std::string sstr(str.Str());
		el.SetText(sstr);
	}
}

void Dialog::DX(std::string& str, int id, bool what)
{
	Window w(GetDlgItem(id));
	if (what)
		w.GetText(str);
	else
		w.SetText(str.c_str());
}

void Dialog::DX(float& val, int id, bool what)
{
	EditLine el(GetDlgItem(id));
	if (what)
	{
		std::string data;
		el.GetText(data);
		TCHAR* ptr;
		val = static_cast<float>(_tcstod(data.c_str(), &ptr));
		if (*ptr != 0)
			_InvalidFormat(FWStr::DoubleExpected, id);
	}
	else
	{
		TCHAR bf[18] = __TEXT("");
		_stprintf(bf, __TEXT("%f"), val);
		int l = _tcslen(bf)-1;
		while ('0' == bf[l])
		{
			bf[l] = 0;
			l--;
		}
		el.SetText(bf);
	}
}


void Dialog::DX(int& val, int id, bool what)
{
	if (what)
	{
		BOOL succ;
		int res = GetDlgItemInt(_hDlg, id, &succ, true);
		if (succ)
			val = res;
		else
			_InvalidFormat(FWStr::IntegerExpected, id);
	}
	else
	{
		SetDlgItemInt(_hDlg, id, val, true);
	}
}

void Dialog::DXComboId(int& val, int id, bool what)
{
	ComboBox cb(GetDlgItem(id));
	if (what)
	{
		int pos = cb.GetCurSel();
		val = cb.GetItemData(pos);
	}
	else
	{
		int pos = cb.FindItemData(val);
		if (-1 != pos)
			cb.SetCurSel(pos);
	}
}



void Dialog::DXButton(bool& val, int id, bool what)
{
	Button button(GetDlgItem(id));
	if (what)
		val = button.IsChecked();
	else
		button.SetCheck(val);
}


