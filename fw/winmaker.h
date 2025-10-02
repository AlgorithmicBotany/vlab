#ifndef __WINMAKER_H__
#define __WINMAKER_H__


class WinMaker
{
public:
	WinMaker(const TCHAR* wc, HINSTANCE hInst = 0);
	void Name(const TCHAR* nm)
	{ cs.lpszName = nm; }
	void MakeChild(int id, HWND hParent)
	{ _AddStyle(WS_CHILD); SetId(id); Owner(hParent); }
	void ClipChildren()
	{ _AddStyle(WS_CLIPCHILDREN); }
	void MakeVisible()
	{ _AddStyle(WS_VISIBLE); }
	void MakeVScroll()
	{ _AddStyle(WS_VSCROLL); }
	void MakeHScroll()
	{ _AddStyle(WS_HSCROLL); }
	void MakeHVScroll()
	{ MakeVScroll(); MakeHScroll(); }
	void MakeClipChildren()
	{ _AddStyle(WS_CLIPCHILDREN); }
	void MakePopup(HWND hOwner)
	{ _AddStyle(WS_POPUP); cs.hwndParent = hOwner; }
	void MakeDlgFrame()
	{ _AddStyle(WS_DLGFRAME); }
	void MakeBorder()
	{ _AddStyle(WS_BORDER); }
	void MakeCaption()
	{ _AddStyle(WS_CAPTION); }
	void MakeMinimizebox()
	{ _AddStyle(WS_MINIMIZEBOX); }
	void MakeSysMenu()
	{ _AddStyle(WS_SYSMENU); }
	void MakeOverlapped()
	{ _AddStyle(WS_OVERLAPPEDWINDOW); }
	void TopMost()
	{ _AddExStyle(WS_EX_TOPMOST); }
	void AcceptFiles()
	{ _AddExStyle(WS_EX_ACCEPTFILES); }

	void Origin(POINT o)
	{
		cs.x = o.x;
		cs.y = o.y;
	}
	void Origin(int x, int y)
	{
		cs.x = x;
		cs.y = y;
	}
	void Size(SIZE sz)
	{
		cs.cx = sz.cx;
		cs.cy = sz.cy;
	}
	void Size(int cx, int cy)
	{
		cs.cx = cx;
		cs.cy = cy;
	}
	void Rectangle(const RECT& r)
	{
		cs.x = r.left;
		cs.y = r.top;
		cs.cx = r.right-r.left;
		cs.cy = r.bottom-r.top;
	}

	void lpData(void* lpV)
	{ cs.lpCreateParams = lpV; }
	void SetId(int id)
	{ 
		assert(0 != (cs.style & WS_CHILD));
		cs.hMenu = reinterpret_cast<HMENU>(id); 
	}
	void SetMenu(HMENU hM)
	{ 
		assert(0 == (cs.style & WS_CHILD));
		cs.hMenu = hM; 
	}
	void SetTitle(const char* title)
	{
		cs.lpszName = title;
	}

	HWND Create() const;
protected:
	void Owner(HWND hParent)
	{ cs.hwndParent = hParent; }
	void _AddStyle(LONG st)
	{ cs.style |= st; }
	void _AddExStyle(DWORD stex)
	{ cs.dwExStyle |= stex; }
private:
	CREATESTRUCT cs;
};


class ButtonMaker : public WinMaker
{
public:
	ButtonMaker(HINSTANCE hInst) : WinMaker(__TEXT("BUTTON"), hInst)
	{}
	void MakePushButton()
	{ _AddStyle(BS_PUSHBUTTON); }
};


#endif
