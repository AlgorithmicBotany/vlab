/**************************************************************************

  File:		dialog.h
  Created:	01-Dec-97


  Declaration of class Dialog


**************************************************************************/


#ifndef __DIALOG_H__
#define __DIALOG_H__


class LongString;


class Dialog
{
public:
	Dialog(WORD, HINSTANCE = 0);
	Dialog(const char*, HINSTANCE = 0);

	int DoModal()
	{ return _DoModal(0); }
	int DoModal(const Window& w) 
	{ return _DoModal(w.Hwnd()); }
	virtual void UpdateData(bool)	// true:	Dialog --> Variables
	{}								// false:	Variables --> Dialog
	void ScreenToClient(RECT& r);
	void SetItemText(int id, const std::string& txt)
	{ ::SetDlgItemText(_hDlg, id, txt.c_str()); }
	void EnableItem(int id, bool enable = true);
	void DisableItem(int id) 
	{ EnableItem(id, false); }
	void ErrorBox(const Exception& e) const
	{ ::MessageBox(_hDlg, e.Msg(), "Error", MB_ICONSTOP); }
	void SetCaption(const std::string& caption) const
	{ SetWindowText(_hDlg, caption.c_str()); }
protected:

	virtual bool HandleMsg(HWND, UINT, WPARAM, LPARAM)
	{ return false; }
	virtual bool DoInit()
	{ return true; }
	virtual HBRUSH CtlColor(HWND, HDC, HWND, int)
	{ return 0; }
	Window GetDlgItem(int id)
	{ return Window(::GetDlgItem(_hDlg, id)); }
	bool InitDialog(HWND);
	void Command(HWND, int, Window, UINT);
	virtual bool Command(int, Window, UINT) 
	{ return false; }
	int _DoModal(HWND);
	void _CheckFailed(UINT, int id = 0);
	void _InvalidFormat(UINT, int);
	static BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
	void EndDialog(int id)
	{ ::EndDialog(Hdlg(), id); }

	virtual bool _Check()
	{ return true; }

	// DX - data exchange functions
	void DX(LongString&, int, bool);
	void DX(std::string&, int, bool);
	void DX(float&, int, bool);
	void DX(int&, int, bool);
	void DXComboId(int&, int, bool);
	void DXButton(bool&, int, bool);

	HWND Hdlg() const
	{ return _hDlg; }
	HINSTANCE HInstance() const
	{ return _hInst; }
private:
	const char* _id;
	HINSTANCE _hInst;
	HWND _hDlg;
	static Dialog* _pCurDlg;
};


#endif
