#include <fw.h>

#include "evalverdlg.h"
#include "resource.h"

EvalVerDlg::EvalVerDlg(const SYSTEMTIME& expire) : 
Dialog(IDD_EVALVER), _msg(256)
{
	ResString frmt(256, IDS_EVALTEXT);
	_seconds = 0;
	TCHAR expdate[16];
	_stprintf
		(
		expdate, 
		__TEXT("%04d/%02d/%02d"), 
		expire.wYear,
		expire.wMonth,
		expire.wDay
		);
	_stprintf
		(
		_msg.Buf(),
		frmt.c_str(),
		expdate
		);
	_msg.CalcLength();
}


bool EvalVerDlg::DoInit()
{
	{
		EditLine Text(GetDlgItem(IDC_TEXT));
		std::string msg(_msg.Str());
		Text.SetText(msg);
	}
	{
		Button Close(GetDlgItem(IDCANCEL));
		Close.Enable(false);
	}
	SetTimer(Hdlg(), 1, 1000, 0);
	return true;
}


bool EvalVerDlg::HandleMsg(HWND, UINT msg, WPARAM, LPARAM)
{
	switch (msg)
	{
	case WM_TIMER :
		{
			_seconds++;
			if (3 == _seconds)
			{
				KillTimer(Hdlg(), 1);
				Button Close(GetDlgItem(IDCANCEL));
				Close.Enable();
			}
		}
		return true;
	default : 
		return false;
	}

}
