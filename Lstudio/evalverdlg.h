#ifndef __EVALVERDLG_H__
#define __EVALVERDLG_H__


class EvalVerDlg : public Dialog
{
public:
	EvalVerDlg(const SYSTEMTIME&);
	bool DoInit();
	bool HandleMsg(HWND, UINT, WPARAM, LPARAM);
private:
	LongString _msg;
	int _seconds;
};

#else
	#error File already included
#endif
