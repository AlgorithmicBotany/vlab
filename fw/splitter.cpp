#include <assert.h>
#include <string>

#include <windows.h>
#include <windowsx.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "window.h"
#include "keystate.h"
#include "sizestate.h"
#include "ownerdraw.h"
#include "ctrl.h"
#include "splitter.h"
#include "wndclass.h"
#include "fwtmplts.h"
#include "gdiobjs.h"
#include "canvas.h"
#include "fwdefs.h"
#include "winmaker.h"

#include "resource.h"

void Splitter::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<Splitter>::Proc);
	wc.style |= CS_GLOBALCLASS;
	wc.hCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_SPLITHORZ));
	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
	wc.Register();
}


HWND Splitter::Create(HWND hParent, HINSTANCE hInst, int childid)
{
	WinMaker maker(_ClassName(), hInst);
	maker.MakeChild(childid, hParent);
	maker.MakeVisible();
	return maker.Create();
}


Splitter::Splitter(HWND hwnd, const CREATESTRUCT* pCS) : Ctrl(hwnd, pCS)
{
	_hParent = pCS->hwndParent;
	_dragging = false;
}


Splitter::~Splitter()
{}


bool Splitter::Paint()
{
	PaintCanvas pc(Hwnd());
	
	{
		ObjectHolder sp(pc, pens3Dset.Light());
		pc.Line(0, 0, Width()-1, 0);
	}
	{
		ObjectHolder sp(pc, pens3Dset.Hilight());
		pc.Line(0, 1, Width()-1, 1);
	}
	{
		ObjectHolder sp(pc, pens3Dset.Shadow());
		pc.Line(0, Height()-2, Width()-1, Height()-2);
	}
	{
		ObjectHolder sp(pc, pens3Dset.DkShadow());
		pc.Line(0, Height()-1, Width()-1, Height()-1);
	}
	return true;
}

bool Splitter::LButtonDown(KeyState, int, int y)
{
	SetCapture(Hwnd());
    // Find x offset of splitter
    // with respect to parent client area
    POINT ptOrg = {0, 0 };
    ClientToScreen(_hParent, &ptOrg);
    int yParent = ptOrg.y;
    ptOrg.y = 0;
    ClientToScreen(Hwnd(), &ptOrg);
    int yChild = ptOrg.y;

    _dragStart = yChild - yParent + Height() / 2 - y;

    _dragY = _dragStart + y;

    // Draw a divider using XOR mode
    UpdateCanvas canvas(_hParent);
	ROp mode(canvas, ROp::NotXorPen);
    canvas.Line(0, _dragY, Width()-1, _dragY);
	_dragging = true;

	return true;
}

bool Splitter::LButtonUp(KeyState, int, int y)
{
	ReleaseCapture();
	FORWARD_MSG_MOVESPLITTER(_hParent, (_dragStart+y), SendMessage);
	_dragging = false;
	return true;
}

bool Splitter::MouseMove(KeyState, int, int y)
{
	if (_dragging)
	{
		// Erase previous divider and draw new one
		UpdateCanvas canvas(_hParent);
		ROp mode(canvas, ROp::NotXorPen);
		canvas.Line(0, _dragY, Width()-1, _dragY);
		_dragY = _dragStart + y;
		canvas.Line(0, _dragY, Width()-1, _dragY);
	}

	return true;
}


bool Splitter::CaptureChanged()
{
	_dragging = false;
    UpdateCanvas canvas(_hParent);
	ROp mode(canvas, ROp::NotXorPen);
    canvas.Line(0, _dragY, Width()-1, _dragY);
	return true;
}
