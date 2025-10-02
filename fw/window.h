/**************************************************************************

  File:		window.h
  Created:	25-Nov-97


  Declaration of class Window


**************************************************************************/


#ifndef __WINDOW_H__
#define __WINDOW_H__


template<typename T>
inline T GetWinLong(HWND hwnd)
{
	return reinterpret_cast<T>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

class LongString;
class RegionBase;
class Menu;
class Icon;
class Font;

class WaitCursor
{
public:
	WaitCursor()
	{
		_hPrev = SetCursor(LoadCursor(0, IDC_WAIT));
	}
	~WaitCursor()
	{
		SetCursor(_hPrev);
	}
private:
	HCURSOR _hPrev;
};



class Window
{
public:
	explicit Window(HWND hwnd = 0) : _hWnd(hwnd) {}

	void ResetHwnd(HWND hwnd)
	{ _hWnd = hwnd; }
	void SetHwnd(HWND hwnd)
	{
		assert(0 == _hWnd);
		assert(0 != hwnd);
		_hWnd = hwnd;
	}
	void Reset(Window w)
	{ _hWnd = w._hWnd; }
	bool IsSet() const
	{ return (_hWnd != 0); }
	bool Is(const Window& w) const
	{ return _hWnd == w._hWnd; }

	void GetRect(RECT& r) const
	{ 
		assert(0 != _hWnd);
		GetClientRect(_hWnd, &r); 
	}
	HWND GetParent() const
	{ return ::GetParent(_hWnd); }
	void GetWindowRect(RECT& r) const
	{
		assert(0 != _hWnd);
		::GetWindowRect(_hWnd, &r);
	}
	RECT WindowRect() const
	{
		assert(IsSet());
		RECT r;
		::GetWindowRect(_hWnd, &r);
		return r;
	}
	void GetCenter(POINT& p) const
	{
		RECT r;
		GetWindowRect(r);
		p.x = (r.left+r.right)/2;
		p.y = (r.top+r.bottom)/2;
	}
	void MoveWindow(int x, int y, int w, int h, bool repaint = true) const
	{ 
		assert(0 != _hWnd);
		::MoveWindow(_hWnd, x, y, w, h, repaint); 
	}
	void MoveWindow(const RECT& r, bool repaint = true) const
	{ 
		assert(0 != _hWnd); 
		::MoveWindow(_hWnd, r.left, r.top, r.right, r.bottom, repaint);
	}
	void ShowAfter(HWND hPrev, const RECT& r) const
	{ ::SetWindowPos(Hwnd(), hPrev, r.left, r.top, r.right, r.bottom, SWP_SHOWWINDOW); }
	void MoveBy(int, int) const;
	void SetText(const std::string& str) const
	{
		assert(0 != _hWnd);
		SetWindowText(_hWnd, str.c_str());
	}
	void SetText(UINT, int len = 64) const;
	void SetFloat(float) const;
	void SetInt(int) const;
	int GetInt() const;
	bool IsEmpty() const
	{ return GetTextLength()==0; }
	void GetText(TCHAR* bf, int len) const
	{ GetWindowText(_hWnd, bf, len); }
	void GetText(LongString&) const;
	void GetText(std::string&) const;
	virtual void Invalidate() const
	{
		assert(0 != _hWnd);
		InvalidateRect(_hWnd, 0, true);
	}
	void Enable(bool enbl = true) const
	{
		assert(0 != _hWnd);
		EnableWindow(_hWnd, enbl);
	}

	void SetMenu(const Menu& menu) const;
	void SetIcon(const Icon&) const;
	void SetRegion(const RegionBase& region) const;
	void ClearRegion() const
	{ ::SetWindowRgn(_hWnd, 0, FALSE); }
	void MessageBox(const std::string&) const;
	void MessageBox(UINT, ...) const;
	void MessageBox(const char*, ...) const;
	bool MessageYesNo(UINT, ...) const;
	bool MessageYesNo(const char*, ...) const;
	void ErrorBox(int, UINT) const;
	void ErrorBox(const Exception& e) const
	{ ::MessageBox(_hWnd, e.Msg(), "Error", MB_ICONSTOP); }
	void GrabFocus() const
	{ ::SetFocus(_hWnd); }
	enum sw
	{
		swHide = SW_HIDE,
		swMaximize = SW_MAXIMIZE,
		swMinimize = SW_MINIMIZE,
		swRestore = SW_RESTORE,
		swShow = SW_SHOW,
		swShowDefault = SW_SHOWDEFAULT,
		swShowMaximized = SW_SHOWMAXIMIZED,
		swShowMinimized = SW_SHOWMINIMIZED,
		swShowMinNoActive = SW_SHOWMINNOACTIVE,
		swShowNA = SW_SHOWNA,
		swShowNoActive = SW_SHOWNOACTIVATE,
		swShowNormal = SW_SHOWNORMAL
	};
	void Show(sw show) const
	{ ::ShowWindow(_hWnd, show); }
	void Show() const
	{ Show(swShow); }
	void Hide() const
	{ Show(swHide); }
	void ShowInFront() const
	{ ::BringWindowToTop(Hwnd()); }
	void SetFont(const Font& font) const;
	ATOM WindowClass() const
	{ return static_cast<ATOM>(GetClassLong(_hWnd, GCW_ATOM)); }
	void PostClose() const
	{ PostMessage(_hWnd, WM_CLOSE, 0, 0); }
	void PostCommand(int cmd, Window w = Window(0), UINT code = 0) const
	{ PostMessage(_hWnd, WM_COMMAND, MAKEWPARAM(cmd, code), reinterpret_cast<LPARAM>(w.Hwnd())); }
	bool IsValid() const
	{ return 0 != IsWindow(Hwnd()); }
	void* GetPtr() const
	{ return reinterpret_cast<void*>(GetWindowLongPtr(_hWnd, GWLP_USERDATA)); }
	void Update() const
	{ ::UpdateWindow(_hWnd); }
	void Capture() const
	{ ::SetCapture(_hWnd); }
	void EndCapture() const
	{ ::ReleaseCapture(); }
	HINSTANCE HInstance() const
	{ return reinterpret_cast<HINSTANCE>(GetWindowLongPtr(_hWnd, GWLP_HINSTANCE)); }
	bool IsEnabled() const
	{ return (0 != IsWindowEnabled(_hWnd)); }
	bool Maximized() const
	{ return (0 != ::IsZoomed(Hwnd())); }
	void SetTimer(UINT_PTR id, UINT elapse) const
	{ ::SetTimer(Hwnd(), id, elapse, 0); }
	void KillTimer(UINT_PTR id) const
	{ ::KillTimer(Hwnd(), id); }
	HWND Hwnd() const
	{ return _hWnd; }
	void Destroy()
	{
		::DestroyWindow(_hWnd);
		_hWnd = 0;
	}
protected:
	int GetTextLength() const
	{ return GetWindowTextLength(_hWnd); }
private:
	HWND _hWnd;
};


class Static : public Window
{
public:
	Static(Window w) : Window(w) {}
};

#endif

