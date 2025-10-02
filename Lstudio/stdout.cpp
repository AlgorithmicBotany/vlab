#include <fw.h>

#include "stdout.h"

/*
INIT_COUNTER(StdOut);

void StdOut::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<StdOut>::Proc);
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.Register();
}


HWND StdOut::Create(HWND hParent, HINSTANCE hInst, int childid)
{
	WinMaker maker(_ClassName(), hInst);
	maker.MakeChild(childid, hParent);
	maker.MakeVisible();
	maker.MakeHVScroll();
	return maker.Create();
}

StdOut::StdOut(HWND hwnd, const CREATESTRUCT*) : Ctrl(hwnd)
{
	_hFont = (HFONT) GetStockObject(ANSI_FIXED_FONT);
	UpdateCanvas cnv(Hwnd());
	ObjectHolder sf(cnv, _hFont);
	TEXTMETRIC tm;
	GetTextMetrics(cnv, &tm);
	_LineHeight = tm.tmHeight;
	_CharWidth = tm.tmAveCharWidth;
	_NumOfLines = 0;
	_FirstLine = 0;
	_FirstCol = 0;
	_MaxFirstLine = 0;
	_aStrings = new LongString[MaxLines];

	// Scrollbars initial settings
	// Vertical
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE | SIF_POS | SIF_DISABLENOSCROLL;
		si.nMin = 0;
		si.nMax = 0;
		si.nPos = 0;
		SetScrollInfo(Hwnd(), SB_VERT, &si, true);
	}
	// Horizontal
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE | SIF_POS | SIF_DISABLENOSCROLL;
		si.nMin = 0;
		si.nMax = 0;
		si.nPos = 0;
		SetScrollInfo(Hwnd(), SB_HORZ, &si, true);
	}
}

StdOut::~StdOut()
{
	delete []_aStrings;
}


bool StdOut::Size(SizeState st, int, int)
{
	if (st.Minimized())
		return true;
	// Vertical scrollbar
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE | SIF_DISABLENOSCROLL;
		si.nPage = Height() / _LineHeight;
		SetScrollInfo(Hwnd(), SB_VERT, &si, true);
	}
	// Horizontal scrollbar
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE | SIF_DISABLENOSCROLL;
		si.nPage = Width() / _CharWidth;
		SetScrollInfo(Hwnd(), SB_HORZ, &si, true);
	}
	return true;
}


bool StdOut::Paint()
{
	PaintCanvas pc(Hwnd());
	ObjectHolder sf(pc, _hFont);
	pc.TextColor(RGB(255, 255, 255));
	pc.BkMode(Canvas::bkTransparent);
	int y = 0;
	for (int i=_FirstLine; i<_NumOfLines; i++)
	{
		pc.TextOut(0, y, _aStrings[i]);
		y += _LineHeight;
	}
	return true;
}


void StdOut::Clear()
{
	_NumOfLines = 0;
	_FirstLine = 0;
	_MaxFirstLine = 0;

	// Adjust vertical scrollbar
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_DISABLENOSCROLL;
		si.nMin = 0;
		si.nMax = _NumOfLines;
		si.nPage = Height() / _LineHeight;
		SetScrollInfo(Hwnd(), SB_VERT, &si, true);
	}
}


void StdOut::AddLine(const TCHAR* line)
{
	if (MaxLines == _NumOfLines)
		return;
	_aStrings[_NumOfLines] = line;
	_NumOfLines++;
	_MaxFirstLine = _NumOfLines - Height()/_LineHeight;
	if (_MaxFirstLine<0)
		_MaxFirstLine = 0;

	// Adjust vertical scrollbar
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_DISABLENOSCROLL;
		si.nMin = 0;
		si.nMax = _NumOfLines;
		si.nPage = Height() / _LineHeight;
		SetScrollInfo(Hwnd(), SB_VERT, &si, true);
	}
	UpdateCanvas cnv(Hwnd());
	ObjectHolder sf(cnv, _hFont);
	cnv.TextColor(RGB(255, 255, 255));
	cnv.BkMode(Canvas::bkTransparent);
	cnv.TextOut(0, _LineHeight*(_NumOfLines-1), _aStrings[_NumOfLines-1]);
}


bool StdOut::VScroll(VScrollCode code, int, HWND)
{
	switch (code)
	{
	case vscLineUp :
		if (_FirstLine>0)
			_VScroll(-1);
		break;
	case vscLineDown :
		if (_MaxFirstLine>_FirstLine)
			_VScroll(1);
		break;
	}
	return true;
}


void StdOut::_VScroll(int i)
{
	_FirstLine+=i;
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = _FirstLine;
		SetScrollInfo(Hwnd(), SB_VERT, &si, true);
	}
	Invalidate();
}
*/
