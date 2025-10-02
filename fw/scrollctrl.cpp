#include <cassert>
#include <string>

#include <windows.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "window.h"
#include "keystate.h"
#include "sizestate.h"
#include "ownerdraw.h"
#include "ctrl.h"
#include "scrollctrl.h"
#include "canvas.h"

Scrollable::Scrollable(HWND hWnd, const CREATESTRUCT* pCS) : Ctrl(hWnd, pCS)
{
	_Scroll.x = _Scroll.y = 0;
	SetBgColor(RGB(0,0,0));
}


bool Scrollable::Size(SizeState, int, int)
{
	POINT p = { 0, 0 };
	if (_Scroll.x > MaxScrollX())
		p.x = MaxScrollX() - _Scroll.x;
	if (_Scroll.y > MaxScrollY())
		p.y = MaxScrollY() - _Scroll.y;

	if (p.x != 0 || p.y != 0)
		ScrollBy(p);

	UpdateScrollbars();
	return true;
}


void Scrollable::ScrollToShow(const RECT& r)
{
	POINT scroll = { 0, 0 };
	if (r.right > ScrollX() + Width())
		scroll.x = r.right - ScrollX() - Width();
	else if (r.left < ScrollX())
		scroll.x = r.left - ScrollX();
	if (r.bottom > ScrollY() + Height())
		scroll.y = r.bottom - ScrollY() - Height();
	else if (r.top < ScrollY())
		scroll.y = r.top - ScrollY();
	ScrollBy(scroll);
}



void Scrollable::ScrollBy(POINT p)
{
	assert(_Scroll.x + p.x >= 0);
	assert
		(
		(_Scroll.x + p.x <= MaxScrollX()) || (MaxScrollX()<0)
		);
	assert(_Scroll.y + p.y >= 0);
	assert
		(
		(_Scroll.y + p.y <= MaxScrollY()) || (MaxScrollY()<0)
		);

	const RECT clip = { 0, 0, Width(), Height() };

	ScrollWindowEx
		(
		Hwnd(),
		-p.x, -p.y,
		0, &clip,
		0, 0, 
		SW_ERASE | SW_INVALIDATE
		);
	_Scroll.x += p.x;
	_Scroll.y += p.y;
	UpdateScrollbars();
}

bool Scrollable::MouseWheel(int i)
{
	const RECT clip = { 0, 0, Width(), Height() };
	i /= 4;

	if (i>_Scroll.y)
		i = _Scroll.y;
	else if (_Scroll.y - i > MaxScrollY())
			i = _Scroll.y - MaxScrollY();

	_Scroll.y -= i;
	ScrollWindowEx
		(
		Hwnd(), 
		0, i,
		0, &clip, 
		0, 0,
		SW_ERASE | SW_INVALIDATE
		);

	UpdateScrollbars();

	return true;
}

bool Scrollable::HScroll(HScrollCode code, int, HWND)
{
	const RECT clip = { 0, 0, Width(), Height() };
	switch (code)
	{
	case hscLineLeft :
		if (_Scroll.x > 0)
		{
			--(_Scroll.x);
			ScrollWindowEx
				(
				Hwnd(), 
				1, 0, 
				0, &clip, 
				0, 0, 
				SW_ERASE | SW_INVALIDATE
				);
			UpdateScrollbars();
		}
		break;
	case hscLineRight :
		if (_Scroll.x < MaxScrollX())
		{
			++(_Scroll.x);
			ScrollWindowEx
				(
				Hwnd(),
				-1, 0,
				0, &clip,
				0, 0,
				SW_ERASE | SW_INVALIDATE
				);
			UpdateScrollbars();
		}
		break;
	case hscPageLeft :
		if (_Scroll.x > 0)
		{
			int scroll = 4*Width()/5;
			if (_Scroll.x < scroll)
				scroll = _Scroll.x;
			_Scroll.x -= scroll;
			Invalidate();
			UpdateScrollbars();
		}
		break;
	case hscPageRight :
		{
			int maxscrx = MaxScrollX();
			if (_Scroll.x < maxscrx)
			{
				int scroll = 4*Width()/5;
				if (scroll+_Scroll.x > maxscrx)
					scroll = maxscrx-_Scroll.x;
				_Scroll.x += scroll;
				Invalidate();
				UpdateScrollbars();
			}
		}
		break;
	case hscThumbPosition :
	case hscThumbTrack :
		{
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_TRACKPOS;
			GetScrollInfo(Hwnd(), SB_HORZ, &si);
			int scroll = si.nTrackPos - _Scroll.x;
			_Scroll.x += scroll;
			ScrollWindowEx
				(
				Hwnd(), 
				-scroll, 0,
				0, &clip, 
				0, 0,
				SW_ERASE | SW_INVALIDATE
				);
			UpdateScrollbars();
		}
		break;
	}
	return true;
}


bool Scrollable::VScroll(VScrollCode code, int, HWND)
{
	const RECT clip = { 0, 0, Width(), Height() };
	switch (code)
	{
	case vscLineUp :
		if (_Scroll.y > 0)
		{
			--(_Scroll.y);
			ScrollWindowEx
				(
				Hwnd(), 
				0, 1,
				0, &clip, 
				0, 0, 
				SW_ERASE | SW_INVALIDATE
				);
			UpdateScrollbars();
		}
		break;
	case vscLineDown :
		if (_Scroll.y < MaxScrollY())
		{
			++(_Scroll.y);
			ScrollWindowEx
				(
				Hwnd(),
				0, -1,
				0, &clip,
				0, 0,
				SW_ERASE | SW_INVALIDATE
				);
			UpdateScrollbars();
		}
		break;
	case vscPageUp :
		if (_Scroll.y > 0)
		{
			int scroll = 4*Height()/5;
			if (_Scroll.y < scroll)
				scroll = _Scroll.y;
			_Scroll.y -= scroll;
			Invalidate();
			UpdateScrollbars();
		}
		break;
	case vscPageDown :
		{
			int maxscry = MaxScrollY();
			if (_Scroll.y < maxscry)
			{
				int scroll = 4*Height()/5;
				if (scroll+_Scroll.y > maxscry)
					scroll = maxscry-_Scroll.y;
				_Scroll.y += scroll;
				Invalidate();
				UpdateScrollbars();
			}
		}
		break;
	case vscThumbPosition :
	case vscThumbTrack :
		{
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_TRACKPOS;
			GetScrollInfo(Hwnd(), SB_VERT, &si);
			int scroll = si.nTrackPos - _Scroll.y;
			_Scroll.y += scroll;
			ScrollWindowEx
				(
				Hwnd(),
				0, -scroll,
				0, &clip,
				0, 0,
				SW_ERASE | SW_INVALIDATE
				);
			UpdateScrollbars();
		}
		break;
	}
	return true;
}

void Scrollable::UpdateScrollbars()
{
	SCROLLINFO si;
	{
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_DISABLENOSCROLL | SIF_POS | SIF_PAGE | SIF_RANGE;
		si.nPos = _Scroll.x;
		si.nPage = Width();
		si.nMin = 0;
		si.nMax = MaxX();
	}
	SetScrollInfo(Hwnd(), SB_HORZ, &si, true);

	{
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_DISABLENOSCROLL | SIF_POS | SIF_PAGE | SIF_RANGE;
		si.nPos = _Scroll.y;
		si.nPage = Height();
		si.nMin = 0;
		si.nMax = MaxY();
	}
	SetScrollInfo(Hwnd(), SB_VERT, &si, true);
}

bool Scrollable::IsVisible(const RECT& r) const
{
	if (r.left < ScrollX())
		return false;
	if (r.top < ScrollY())
		return false;
	if (r.right > ScrollX() + Width())
		return false;
	if (r.bottom > ScrollY() + Height())
		return false;
	return true;
}


bool Scrollable::Paint()
{
	PaintCanvas cnv(Hwnd(), BgColor());
	cnv.SetOrigin(-ScrollX(), -ScrollY());
	const RECT r = { ScrollX(), ScrollY(), ScrollX()+Width(), ScrollY()+Height() };
	DoPaint(cnv, r);
	return true;
}
