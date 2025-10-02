/**************************************************************************

  File:		Editline.h
  Created:	02-Dec-97


  Declaration of class EditLine


**************************************************************************/


#ifndef __EDITLINE_H__
#define __EDITLINE_H__


class EditLine : public Window
{
public:
	EditLine() {}
	EditLine(Window w) : Window(w)
	{}
	int GetLineCount() const
	{ return Edit_GetLineCount(Hwnd()); }
	int LineIndex(int l) const
	{ return Edit_LineIndex(Hwnd(), l); }
	int LineLength(int l) const
	{ return Edit_LineLength(Hwnd(), l); }
	int GetFirstVisibleLine() const
	{ return Edit_GetFirstVisibleLine(Hwnd()); }
	void GetLine(int ln, std::string& bf) const
	{ 
		const int lix = LineIndex(ln);
		const int len = LineLength(lix);
		bf.reserve(len+2);
		bf.resize(len+1);
		Edit_GetLine(Hwnd(), ln, &(bf[0]), len+1); 
		bf.resize(len);
	}
	void SetModify(bool modify) const
	{ Edit_SetModify(Hwnd(), modify); }
	void SetSel(int ss, int es) const
	{ Edit_SetSel(Hwnd(), ss, es); }
	void ScrollCaret() const
	{ Edit_ScrollCaret(Hwnd()); }
	int LineFromChar(int ch) const
	{ return Edit_LineFromChar(Hwnd(), ch); }
	void ClearSel() const
	{ SetSel(-1, -1); }
	void EditAtEnd() const
	{
		int l = GetTextLength();
		SetSel(l, l);
	}
	bool GetModify() const
	{ return 0 != SendMessage(Hwnd(), EM_GETMODIFY, 0, 0); }
	bool SetReadOnly(bool on) const
	{ return 0 != Edit_SetReadOnly(Hwnd(), on ? TRUE : FALSE); }
	void GetSel(DWORD* pSt, DWORD* pEn) const
	{ SendMessage(Hwnd(), EM_GETSEL, (WPARAM)(pSt), (LPARAM)(pEn)); }
	void ReplaceSelection(const std::string& txt) const
	{ SendMessage(Hwnd(), EM_REPLACESEL, (WPARAM) TRUE, (LPARAM)(txt.c_str())); }
	void Copy() const
	{ SendMessage(Hwnd(), WM_COPY, 0, 0); }
	void SetTextLimit(int l)
	{ SendMessage(Hwnd(), EM_SETLIMITTEXT, l, 0); }
	int GetTextLimit() const
	{ return static_cast<int>(SendMessage(Hwnd(), EM_GETLIMITTEXT, 0, 0)); }
	int GetCurrentLine() const
	{
		DWORD st;
		GetSel(&st, 0);
		return LineFromChar(st);
	}
	void SetCurrentLine(int ln) const
	{ 
		int li = LineIndex(ln);
		SetSel(li, li);
	}
	void Scroll(WPARAM vsc)
	{ SendMessage(Hwnd(), EM_SCROLL, vsc, 0); }
};

#endif
