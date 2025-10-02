#ifndef __TEXTEDIT_H__
#define __TEXTEDIT_H__

#include "finddlg.h"

template<class DefaultName, UINT idd>
class TextEdit : public FormCtrl
{
public:
	TextEdit(HWND hwnd) :
	FormCtrl(hwnd),
	_Status(GetDlgItem(IDC_STATUS)),
	_Edit(GetDlgItem(IDC_EDIT)),
	_Find(GetDlgItem(IDC_FINDTXT)),
	_FNextBtn(GetDlgItem(IDC_FINDNEXT)),
	_FPrevBtn(GetDlgItem(IDC_FINDPREV)),
	_FNext(App::GetInstance(), MAKEINTRESOURCE(IDI_FINDNEXT)),
	_FPrev(App::GetInstance(), MAKEINTRESOURCE(IDI_FINDPREV)),
	_SubclassEdit(_Edit.Hwnd(), &TextEdit::_EditWndProc, this),
	_FindMatchCase(true),
	_tooltips(Hwnd(), App::GetInstance())
	{
		HINSTANCE hInst = App::GetInstance();

		_Filename[0] = 0;
		_FindPattern[0] = 0;
		SetDlgItemInt(Hwnd(), IDC_GOTO, 1, true);

		_Edit.SetTextLimit(0x80000);

		_FNextBtn.SetIcon(_FNext);
		_tooltips.Add(_FNextBtn, IDS_FINDNEXT, hInst);
		_FNextBtn.Enable(false);

		_FPrevBtn.SetIcon(_FPrev);
		_tooltips.Add(_FPrevBtn, IDS_FINDPREV, hInst);
		_FPrevBtn.Enable(false);
		_FPrevBtn.Hide();

	}
	~TextEdit()
	{}

	static HWND Create(HWND hParent, HINSTANCE hInst)
	{
		FormCreator<TextEdit<DefaultName, idd> > creator;
		return CreateDialogParam
			(
			hInst,
			MAKEINTRESOURCE(idd),
			hParent,
			reinterpret_cast<DLGPROC>(FormCtrl::DlgProc),
			reinterpret_cast<LPARAM>(&creator)
			);
	}

	bool Size(SizeState, int w, int h)
	{
		RECT r;

		_Edit.GetWindowRect(r);

		// Rewrite to POINTs
		POINT pts[2];
		pts[0].x = r.left; pts[0].y = r.top;
		pts[1].x = r.right; pts[1].y = r.bottom;

		// Map to AnyTextEdit window
		MapWindowPoints(HWND_DESKTOP, Hwnd(), pts, 2);

		// Back to RECT but with right and bottom margin
		r.left = pts[0].x; r.top = pts[0].y;
		r.right = w - 2*r.left; r.bottom = h - r.top - 2*r.left;

		// And move
		_Edit.MoveWindow(r);

		return true;
	}

	bool SetFocus(HWND)
	{
		_Edit.GrabFocus();
		return true;
	}

	bool Command(int id, Window ctl, UINT notify)
	{
		switch (id)
		{
		case IDC_GOTO :
			if (EN_SETFOCUS == notify)
				_InitGoto();
			else if (EN_KILLFOCUS == notify)
				App::ClearModeless();
			break;
		case IDC_FINDTXT :
			if (EN_SETFOCUS == notify)
				App::SetModeless(Hwnd());
			else if (EN_KILLFOCUS == notify)
				App::ClearModeless();
			else if (EN_CHANGE == notify)
			{
				_FNextBtn.Enable(!ctl.IsEmpty());
			}  
			break;
		case IDC_FINDNEXT :
			_FindNext();
			break;
		case IDC_FINDPREV :
			_FindPrev();
			break;
		case IDC_GO :
			_Goto();
			break;
		case IDC_FNDLG :
			Find();
			break;
		}

		return true;
	}

	HBRUSH CtlColorEdit(HDC hDC, HWND hCtl)
	{
		if (_Edit.Is(Window(hCtl)))
		{
			TmpCanvas cnv(hDC);
			cnv.TextColor(options.GetTextEditTxtColor());
			cnv.BkColor(options.GetTextEditBgColor());
			return options.GetTextEditBgBrush();
		}
		else
			return 0;
	}

	void Import(const TCHAR* fname)
	{
		ReadTextFile src(fname);

		_tcscpy(_Filename, fname);

	#ifdef UNICODE
		#error This piece of code must be rewritten not using LongString in UNICODE version
	#endif

		std::string contents;

		std::string line;

		for (;;)
		{
			src.Read(line);
			contents += line;
			if (src.Eof())
				break;
			else
				contents += "\r\n";
		}
		_Edit.SetText(contents);

	#ifdef UNICODE
		#error End of code not portable to UNICODE 
	#endif

		_Status.SetText(fname);
		_UpdateLine();
	}

	void Generate() const
	{
		WriteTextFile trg(Name());
		const int lines = _Edit.GetLineCount();
		std::string line;
		for (int l=0; l<lines; l++)
		{
			_Edit.GetLine(l, line);
			if (!line.empty())
			{
				trg.Write(line.c_str());
			}
			if (l<lines-1)
				trg.Write("\n");
		}
		_Edit.SetModify(false);
	}

	void Clear()
	{
		_Edit.SetText("");
		_Status.SetText(DefaultName::Name());
		_Filename[0] = 0;
		_FindPattern[0] = 0;
		_UpdateLine();
	}

	void Show()
	{
		FormCtrl::Show();
		_Edit.GrabFocus();
	}

	void SetEditorFont(const Font& font)
	{
		_Edit.SetFont(font);
	}

	bool IsNamed() const
	{ return 0 != _Filename[0]; }
	const char* Name() const
	{
		if (0 != _Filename[0])
			return _Filename;
		else
			return DefaultName::Name();
	}

	EditLine HEdit() const
	{ return _Edit; }
	void Find()
	{
		_GetTextToFind();
		FindDialog dlg(_FindPattern, _FindMatchCase, _Edit);
		switch(dlg.DoModal(*this))
		{
		case -1 :
			MessageBox(IDERR_PATTERNNOTFOUND);
			break;
		case IDOK :
			_Find.GrabFocus();
			_Edit.SetSel(dlg.StartSel(), dlg.EndSel());
			_Edit.ScrollCaret();
			_UpdateLine();
			break;
		}
		_tcsncpy(_FindPattern, dlg.FindPattern().c_str(), eMaxFindPattern+1);
		_FindPattern[eMaxFindPattern] = 0;
		_FindMatchCase = dlg.MatchCase();
		_Find.SetText(_FindPattern);
	}

	void FindAgain()
	{
		if (0 == _FindPattern[0])
			Find();
		else
		{
			FindDialog dlg(_FindPattern, _FindMatchCase, _Edit);
			if (dlg.JustFind())
			{
				_Edit.SetSel(dlg.StartSel(), dlg.EndSel());
				_Edit.ScrollCaret();
				_Edit.GrabFocus();
				_UpdateLine();
			}
			else
				MessageBox(IDERR_PATTERNNOTFOUND);
		}
	}

	void FindPrev() {}
	bool IsEmpty() const
	{
		return _Edit.IsEmpty();
	}

	int GetSelectionSize() const
	{
		DWORD st, en;
		_Edit.GetSel(&st, &en);
		return en-st;
	}

	void GetSelection(std::string& bf) const
	{
		int sz = GetSelectionSize();
		if (sz>0)
		{
			bf.reserve(sz+1);
			bf.resize(sz);
			_Edit.Copy();
			UseClipboard clpbrd(Hwnd());
			HGLOBAL hMem = GetClipboardData(CF_TEXT);
			MemoryLock lock(hMem);
			strcpy(&(bf[0]), reinterpret_cast<const char*>(lock.Ptr()));
		}
		else
			bf = "";
	}

	void SelectAfterSelection() const
	{
		DWORD en;
		_Edit.GetSel(0, &en);
		_Edit.SetSel(en, en);
	}

	void ReplaceSelection(const TCHAR* txt) const
	{
		_Edit.ReplaceSelection(txt);
	}

	void EnableEdit(bool enable)
	{ _Edit.Enable(enable); }


protected:

	enum
	{ eMaxFindPattern = 32 };

	void _InitGoto()
	{
		int ln = _Edit.LineFromChar(-1);
		SetDlgItemInt(Hwnd(), IDC_GOTO, ln+1, true);
		_Edit.ClearSel();
		App::SetModeless(Hwnd());
	}

	void _Goto()
	{
		Window w(GetFocus());
		if (w.Is(GetDlgItem(IDC_GOTO)))
		{
			int line = GetDlgItemInt(Hwnd(), IDC_GOTO, 0, TRUE) - 1;
			_Edit.GrabFocus();
			int lc = _Edit.GetLineCount();
			if (line>lc)
				line = lc;
			int li = _Edit.LineIndex(line);
			_Edit.SetSel(li, li);
			_Edit.ScrollCaret();
		}
		else if (w.Is(_Find))
		{
			_GetTextToFind();
			if (0 != _FindPattern[0])
				FindAgain();
			else
				MessageBeep(0xFFFFFFFF);
		}
	}

	void _FindNext()
	{
		_GetTextToFind();
		if (0 != _FindPattern[0])
			FindAgain();
		else
			MessageBeep(0xFFFFFFFF);
	}

	void _FindPrev()
	{
		_GetTextToFind();
		if (0 != _FindPattern[0])
			FindPrev();
		else
			MessageBeep(0xFFFFFFFF);
	}

	Static _Status;
	EditLine _Edit;
	EditLine _Find;
	Button _FNextBtn;
	Button _FPrevBtn;

	Icon _FNext;
	Icon _FPrev;

	Subclass<TextEdit> _SubclassEdit;
	TCHAR _Filename[_MAX_PATH+1];
	TCHAR _FindPattern[eMaxFindPattern+1];
	bool _FindMatchCase;

	void _GetTextToFind()
	{
		std::string tofind;
		_Find.GetText(tofind);
		_tcsncpy(_FindPattern, tofind.c_str(), eMaxFindPattern);
		_FindPattern[eMaxFindPattern] = 0;
	}

	LRESULT _EditWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT res = CallWindowProc(_SubclassEdit.WndProc(), hWnd, msg, wParam, lParam);
		switch (msg)
		{
		case WM_KEYDOWN :
		case WM_LBUTTONDOWN :
		case WM_LBUTTONUP :
		case WM_CHAR :
			_UpdateLine();
			break;
		}
		return res;
	}

	void _UpdateLine()
	{
		int ln = _Edit.LineFromChar(-1)+1;
		SetDlgItemInt(Hwnd(), IDC_GOTO, ln, true);
	}

	ToolTip _tooltips;
};


#else
	#error File already included
#endif
