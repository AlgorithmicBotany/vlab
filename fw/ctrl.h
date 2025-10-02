/**************************************************************************

  File:		ctrl.h
  Created:	24-Nov-97


  Declaration of class Ctrl


**************************************************************************/


#ifndef __CTRL_H__
#define __CTRL_H__

class Ctrl : public Window
{
public:
	Ctrl(HWND, const CREATESTRUCT*);
	virtual ~Ctrl();


	enum HScrollCode
	{
		hscEndScroll = SB_ENDSCROLL,
		hscLeft = SB_LEFT,
		hscRight = SB_RIGHT,
		hscLineLeft = SB_LINELEFT,
		hscLineRight = SB_LINERIGHT,
		hscPageLeft = SB_PAGELEFT,
		hscPageRight = SB_PAGERIGHT,
		hscThumbPosition = SB_THUMBPOSITION,
		hscThumbTrack = SB_THUMBTRACK
	};
	enum VScrollCode
	{
		vscEndScroll = SB_ENDSCROLL,
		vscBottom = SB_BOTTOM,
		vscTop = SB_TOP,
		vscLineDown = SB_LINEDOWN,
		vscLineUp = SB_LINEUP,
		vscPageDown = SB_PAGEDOWN,
		vscPageUp = SB_PAGEUP,
		vscThumbPosition = SB_THUMBPOSITION,
		vscThumbTrack = SB_THUMBTRACK
	};
	enum ActiveState
	{
		asActive = WA_ACTIVE,
		asClickActive = WA_CLICKACTIVE,
		asInactive = WA_INACTIVE
	};
	
	static void OnDestroy(HWND);

	virtual bool Activate(ActiveState, HWND)
	{ return false; }
	virtual bool CaptureChanged()
	{ return false; }
	virtual bool Char(TCHAR)
	{ return false; }

	// returning false prevents the window from being destroyed
	virtual bool Close()
	{ return true; }
	virtual bool Command(int, Window, UINT)
	{ return false; }
	virtual void ContextMenu(HWND, UINT, UINT)
	{}
	virtual HBRUSH CtlColorEdit(HDC, HWND)
	{ return 0; }
	virtual bool DrawItem(OwnerDraw::Draw) 
	{ return false; }
	virtual bool EraseBackground(HDC)
	{ return false; }
	virtual int GetDlgCode() 
	{ return 0; }
	virtual bool HScroll(HScrollCode, int, HWND)
	{ return false; }
	virtual bool InitMenu(MenuManipulator)
	{ return false; }
	virtual bool KeyDown(UINT)
	{ return false; }
	virtual bool KillFocus(HWND)
	{ return false; }
	virtual bool LButtonDown(KeyState, int, int)
	{ return false; }
	virtual bool LButtonUp(KeyState, int, int)
	{ return false; }
	virtual bool LBDblClick(KeyState, int, int)
	{ return false; }
	virtual bool MButtonDown(KeyState, int, int)
	{ return false; }
	virtual bool MButtonUp(KeyState, int, int)
	{ return false; }
	virtual bool MeasureItem(OwnerDraw::Measure)
	{ return false; }
	virtual bool MouseActivate(LRESULT&)
	{ return false; }
	virtual bool MouseMove(KeyState, int, int)
	{ return false; }
	virtual bool MouseWheel(int)
	{ return false; }
	bool OnMove(int x, int y)
	{
		_pos.x = x;
		_pos.y = y;
		return Move(x, y);
	}
	virtual bool Move(int, int)
	{ return false; }
	virtual bool Moving(RECT*)
	{ return false; }
	virtual bool NCHitTest(int, int, LRESULT&)
	{ return false; }
	virtual LRESULT Notify(int, const NMHDR*)
	{ return 0; }
	virtual bool Paint()
	{ return false; }
	virtual bool QueryEndSession()
	{ return true; }
	virtual bool RButtonDown(KeyState, int, int)
	{ return false; }
	bool OnSize(SizeState st, int w, int h)
	{
		_size.cx = w;
		_size.cy = h;
		return Size(st, w, h);
	}
	virtual bool Size(SizeState, int, int)
	{ return false; }
	virtual bool Timer(UINT)
	{ return false; }
	virtual bool VScroll(VScrollCode, int, HWND)
	{ return false; }
	virtual bool SetFocus(HWND)
	{ return false; }

	int Width() const
	{ return _size.cx; }
	int Height() const
	{ return _size.cy; }
	
	int PosX() const
	{ return _pos.x; }
	int PosY() const
	{ return _pos.y; }

	void MoveBy(int dx, int dy)
	{
		RECT r; 
		r.left = PosX() + dx; r.top = PosY() + dy;
		r.right = Width(); r.bottom = Height();
		MoveWindow(r);
	}
	void MoveLeftTo(int x)
	{
		RECT wr;
		GetWindowRect(wr);
		RECT r;
		r.left = x;
		r.top = wr.top;
		r.right = (wr.right-wr.left)-(x-wr.left);
		r.bottom = wr.bottom-wr.top;
		MoveWindow(r);
	}
	void MoveRightTo(int x)
	{
		RECT wr;
		GetWindowRect(wr);
		RECT r;
		r.left = wr.left;
		r.top = wr.top;
		r.right = x-wr.left;
		r.bottom = wr.bottom-wr.top;
	}

private:
	SIZE _size;
	POINT _pos;
};

#endif
