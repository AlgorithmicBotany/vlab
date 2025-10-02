/**************************************************************************

  File:		canvas.h
  Created:	01-Dec-97


  Declaration of class Canvas


**************************************************************************/


#ifndef __CANVAS_H__
#define __CANVAS_H__

class Window;

class Canvas
{
public:
	void TextOut(int x, int y, const std::string& str)
	{ ::TextOut(_hDC, x, y, str.c_str(), str.length()); }
	void CharOut(int x, int y, TCHAR c)
	{ ::TextOut(_hDC, x, y, &c, 1); }
	void TabbedTextOut(int x, int y, const std::string& str)
	{ ::TabbedTextOut(_hDC, x, y, str.c_str(), str.length(), 0, 0, 0); }
	void SetRedPixel(int x, int y)
	{ ::SetPixel(_hDC, x, y, RGB(255, 0, 0)); }
	void SetBlackPixel(int x, int y)
	{ ::SetPixel(_hDC, x, y, RGB(0, 0, 0)); }
	void MoveTo(int x, int y)
	{ ::MoveToEx(_hDC, x, y, 0); }
	void LineTo(int x, int y)
	{ ::LineTo(_hDC, x, y); }
	operator HDC() const
	{ return _hDC; }
	void GetTextMetrics(TEXTMETRIC& tm)
	{ ::GetTextMetrics(_hDC, &tm); }
	void DrawText(const std::string& str, RECT* pRect, UINT flags)
	{ ::DrawText(_hDC, str.c_str(), str.length(), pRect, flags); }
	void Line(int x1, int y1, int x2, int y2)
	{
		::MoveToEx(_hDC, x1, y1, 0);
		::LineTo(_hDC, x2, y2);
	}
	void Rectangle(int x1, int y1, int x2, int y2)
	{
		MoveTo(x1, y1);
		LineTo(x2, y1);
		LineTo(x2, y2);
		LineTo(x1, y2);
		LineTo(x1, y1);
	}
	void Rectangle(RECT r)
	{
		MoveTo(r.left, r.top);
		LineTo(r.left, r.bottom);
		LineTo(r.right, r.bottom);
		LineTo(r.right, r.top);
		LineTo(r.left, r.top);
	}
	void Rectangle(POINT p1, POINT p2)
	{
		MoveTo(p1.x, p1.y);
		LineTo(p1.x, p2.y);
		LineTo(p2.x, p2.y);
		LineTo(p2.x, p1.y);
		LineTo(p1.x, p1.y);
	}
	void FilledRectangle(int x1, int y1, int x2, int y2)
	{
		::Rectangle(_hDC, x1, y1, x2, y2);
	}
	void FilledRectangle(const RECT& r)
	{
		::Rectangle(_hDC, r.left, r.top, r.right, r.bottom); 
	}
	void FilledRectangle(const RECT& r, COLORREF clr)
	{
		SetBkColor(_hDC, clr);
		ExtTextOut(_hDC, 0, 0, ETO_OPAQUE, &r, 0, 0, 0);
	}
	enum taH
	{
		taHNone = 0,
		taLeft = TA_LEFT,
		taRight = TA_RIGHT,
		taCenter = TA_CENTER
	};
	enum taV
	{
		taVNone = 0,
		taBaseline = TA_BASELINE,
		taBottom = TA_BOTTOM,
		taTop = TA_TOP
	};
	void TextAlign(taV tav, taH tah)
	{ ::SetTextAlign(_hDC, tav | tah); }
	void Circle(int x, int y, int rad)
	{
		assert(rad>1);
		::Ellipse(_hDC, x-rad, y-rad, x+rad, y+rad);
	}
	void Ellipse(int left, int top, int right, int bottom)
	{
		::Ellipse(_hDC, left, top, right, bottom);
	}
	void Set3x3Pixel(int x, int y, COLORREF rgb)
	{
		::SetPixel(_hDC, x-1, y-1, rgb);
		::SetPixel(_hDC, x, y-1, rgb);
		::SetPixel(_hDC, x+1, y-1, rgb);
		::SetPixel(_hDC, x-1, y, rgb);
		::SetPixel(_hDC, x, y, rgb);
		::SetPixel(_hDC, x+1, y, rgb);
		::SetPixel(_hDC, x-1, y+1, rgb);
		::SetPixel(_hDC, x, y+1, rgb);
		::SetPixel(_hDC, x+1, y+1, rgb);
	}
	void MeasureText(const std::string& str, SIZE& Sz) const
	{ GetTextExtentPoint32(_hDC, str.c_str(), str.length(), &Sz); }
	void DrawIcon(HICON hIcon, int x, int y) const
	{ ::DrawIconEx(_hDC, x, y, hIcon, 0, 0, 0, 0, DI_NORMAL); }

	enum bkMode
	{
		bkTransparent = TRANSPARENT,
		bkOpaque = OPAQUE
	};
	void BkMode(bkMode mode)
	{ ::SetBkMode(_hDC, mode); }
	void TextColor(COLORREF clr)
	{ ::SetTextColor(_hDC, clr); }
	void BkColor(COLORREF clr)
	{ ::SetBkColor(_hDC, clr); }
	void DrawBitmap(HBITMAP hBmp, int x, int y, int cx, int cy, HDC hMem)
	{
		SelectObject(hMem, hBmp);
		BitBlt(_hDC, x, y, cx, cy, hMem, 0, 0, SRCCOPY);
	}
	void SetOrigin(int x, int y)
	{ 
		::SetViewportOrgEx(_hDC, x, y, 0);
	}

	void ClipRect(int l, int t, int r, int b)
	{
		ClearClip();
		::IntersectClipRect(_hDC, l, t, r, b);
	}
	void ClearClip()
	{
		::SelectClipRgn(_hDC, 0);
	}
protected:
	Canvas() : _hDC(0) {}
	void SetDC(HDC hdc)
	{ _hDC = hdc; }
	HDC GetDC() const
	{ return _hDC; }
private:
	HDC _hDC;
};


class PaintCanvas : public Canvas
{
public:
	PaintCanvas(HWND hwnd) : _hWnd(hwnd)
	{
		SetDC(BeginPaint(_hWnd, &_ps));
	}
	PaintCanvas(HWND hwnd, COLORREF bgclr) : _hWnd(hwnd)
	{
		SetDC(BeginPaint(_hWnd, &_ps));
		if (_ps.fErase)
			FilledRectangle(_ps.rcPaint, bgclr);
	}
	~PaintCanvas()
	{
		EndPaint(_hWnd, &_ps);
	}
private:
	const HWND _hWnd;
	PAINTSTRUCT _ps;
};


class UpdateCanvas : public Canvas
{
public:
	UpdateCanvas(HWND hwnd) : _hWnd(hwnd)
	{
		SetDC(::GetDC(_hWnd));
	}
	UpdateCanvas(Window*);
	~UpdateCanvas()
	{
		ReleaseDC(_hWnd, GetDC());
	}
private:
	const HWND _hWnd;
};

class DesktopCanvas : public Canvas
{
public:
	DesktopCanvas()
	{ SetDC(::GetDC(NULL)); }
	~DesktopCanvas()
	{ ::ReleaseDC(NULL, GetDC()); }
};

class MemoryCanvas : public Canvas
{
public:
	MemoryCanvas(HDC hdc)
	{
		SetDC(CreateCompatibleDC(hdc));
	}
	~MemoryCanvas()
	{
		DeleteDC(GetDC());
	}
private:
};


class TmpCanvas : public Canvas
{
public:
	TmpCanvas(HDC hdc) 
	{ SetDC(hdc); }
};


class ObjectHolder
{
public:
	ObjectHolder(HDC hdc, HGDIOBJ hObj) : _hdc(hdc)
	{
		_hPrevObj = SelectObject(_hdc, hObj);
	}
	~ObjectHolder()
	{
		SelectObject(_hdc, _hPrevObj);
	}
private:
	const HDC _hdc;
	HGDIOBJ _hPrevObj;
};


#endif

