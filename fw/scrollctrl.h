#ifndef __SCROLLCTRL_H__
#define __SCROLLCTRL_H__

class Canvas;

class Scrollable : public Ctrl
{
public:
	Scrollable(HWND, const CREATESTRUCT*);
	bool HScroll(HScrollCode, int, HWND);
	bool VScroll(VScrollCode, int, HWND);
	bool MouseWheel(int);
	bool Size(SizeState, int, int);
	bool Paint();
	void SetScroll(int x, int y)
	{
		_Scroll.x = x;
		_Scroll.y = y;
	}
	void SetScrollX(int x)
	{ _Scroll.x = x; }
	void SetScrollY(int y)
	{ _Scroll.y = y; }
	void ScrollBy(POINT p);
	void ScrollToShow(const RECT&);
	bool IsVisible(const RECT&) const;
	int ScrollX() const
	{ return _Scroll.x; }
	int ScrollY() const
	{ return _Scroll.y; }
	void UpdateScrollbars();
	COLORREF BgColor() const
	{ return _BgColor; }
	void SetBgColor(COLORREF rgb)
	{ _BgColor = rgb; }
protected:
	virtual int MaxScrollX() const = 0;
	virtual int MaxScrollY() const = 0;
	virtual int MaxX() const = 0;
	virtual int MaxY() const = 0;
	virtual void DoPaint(Canvas&, const RECT&) = 0;
private:
	POINT _Scroll;
	COLORREF _BgColor;
};

#endif
