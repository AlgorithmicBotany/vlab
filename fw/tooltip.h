#ifndef __TOOLTIP_H__
#define __TOOLTIP_H__

class ToolTip
{
public:
	ToolTip(HWND hOwner, HINSTANCE hInst);
	~ToolTip()
	{ DestroyWindow(_hTT); }
	void Add(Window w, WORD textId, HINSTANCE hInst);
	void Add(const Window&, int, const RECT&);
	void Clear();
	bool Is(HWND hWnd) const
	{ return (_hTT == hWnd); }
private:
	HWND _hTT;
};


#endif
