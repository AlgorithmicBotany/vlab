#include <cassert>

#include <string>

#include <windows.h>
#include <commctrl.h>

#include "warningset.h"

#include "fwdefs.h"
#include "exception.h"
#include "menu.h"
#include "window.h"
#include "keystate.h"
#include "sizestate.h"
#include "ownerdraw.h"
#include "ctrl.h"
#include "formctrl.h"
#include "trackbarctrl.h"
#include "dialog.h"
#include "owndrwbtn.h"
#include "gdiobjs.h"
#include "canvas.h"
#include "colors.h"
#include "pickclrdlg.h"

OwnerDrawButton::OwnerDrawButton(COLORREF clr, ColorChangedCallback* pCB) :
_cb(pCB)
{
	_FillClr = clr;
	_checked = false;
	_r.left = _r.right = _r.bottom = _r.top = 0;
	_hWnd = 0;
	_hPickColor = 0;
}


void OwnerDrawButton::SetColor(COLORREF clr)
{
	_FillClr = clr;
}


void OwnerDrawButton::DrawItem(OwnerDraw::Draw& ds) const
{
	assert(ds.IsButton());
	if (ds.DrawEntire())
	{
		_DrawFace(ds);
		_DrawSelect(ds);
		_DrawFocus(ds);
	}
	else
	{
		if (ds.DrawFocus())
			_DrawFocus(ds);
		if (ds.DrawSelect())
			_DrawSelect(ds);
	}
}


void OwnerDrawButton::_DrawFace(OwnerDraw::Draw& ds) const
{
	RECT r =
	{
		ds.ItemRect().left+2,
			ds.ItemRect().top+2,
			ds.ItemRect().right-1,
			ds.ItemRect().bottom-1
	};

	{
		Brush fl(GetSysColor(COLOR_3DFACE));
		FillRect(ds.DC(), &r, fl);
	}

	r.left += 2;
	r.top += 2;
	r.right -= 3;
	r.bottom -= 3;

	{
		Brush fl(_FillClr);
		FillRect(ds.DC(), &r, fl);
	}
}



void OwnerDrawButton::_DrawSelect(OwnerDraw::Draw& ds) const
{
	HGDIOBJ pens[4];
	if (ds.IsSelected() || _checked)
	{
		pens[3] = pens3Dset.DkShadow();
		pens[2] = pens3Dset.Shadow();
		pens[1] = pens3Dset.Hilight();
		pens[0] = pens3Dset.Light();
	}
	else
	{
		pens[0] = pens3Dset.DkShadow();
		pens[1] = pens3Dset.Shadow();
		pens[2] = pens3Dset.Hilight();
		pens[3] = pens3Dset.Light();
	}

	HDC hdc = ds.DC();
	const RECT& r = ds.ItemRect();
	{
		ObjectHolder sp(hdc, pens[0]);
		MoveToEx(hdc, r.left, r.bottom-1, 0);
		LineTo(hdc, r.right-1, r.bottom-1);
		LineTo(hdc, r.right-1, r.top-1);
	}
	{
		ObjectHolder sp(hdc, pens[1]);
		MoveToEx(hdc, r.left+1, r.bottom-2, 0);
		LineTo(hdc, r.right-2, r.bottom-2);
		LineTo(hdc, r.right-2, r.top);
	}
	{
		ObjectHolder sp(hdc, pens[2]);
		MoveToEx(hdc, r.left, r.bottom-2, 0);
		LineTo(hdc, r.left, r.top);
		LineTo(hdc, r.right-1, r.top);
	}
	{
		ObjectHolder sp(hdc, pens[3]);
		MoveToEx(hdc, r.left+1, r.bottom-3, 0);
		LineTo(hdc, r.left+1, r.top+1);
		LineTo(hdc, r.right-2, r.top+1);
	}


}


void OwnerDrawButton::_DrawFocus(OwnerDraw::Draw& ds) const
{
	if (ds.DrawFocus())
	{
		const RECT r = 
		{ 
			ds.ItemRect().left+4, 
				ds.ItemRect().top+4, 
				ds.ItemRect().right-4,
				ds.ItemRect().bottom-4 
		};
		DrawFocusRect(ds.DC(), &r);
	}
	else
	{
		ObjectHolder sfp(ds.DC(), pens3Dset.Face());
		MoveToEx(ds.DC(), ds.ItemRect().left+4, ds.ItemRect().top+4, 0);
		LineTo(ds.DC(), ds.ItemRect().right-5, ds.ItemRect().top+4);
		LineTo(ds.DC(), ds.ItemRect().right-5, ds.ItemRect().bottom-5);
		LineTo(ds.DC(), ds.ItemRect().left+4, ds.ItemRect().bottom-5);
		LineTo(ds.DC(), ds.ItemRect().left+4, ds.ItemRect().top+4);
	}
}



void OwnerDrawButton::SelectColor()
{
	if (0 == _hPickColor)
	{	
		PickColorModeless::Callbacks cb(_ColorChangedClbck, _PickClosedClbck, this);
		static TCHAR lbl[64];
		assert(0 != _hWnd);
		GetWindowText(_hWnd, lbl, 64);
		PickColorModeless::InitData init(&cb, lbl, Color(), _r);
		_hPickColor = PickColorModeless::Create(_hWnd, &init);
		Check();
	}
	else
	{
		DestroyWindow(_hPickColor);
		_hPickColor = 0;
	}
}



void OwnerDrawButton::_DoColorChanged(COLORREF clr)
{
	SetColor(clr);
	assert(0 != _hWnd);
	InvalidateRect(_hWnd, 0, true);
	if (_cb._pClbck != 0)
		(_cb._pClbck)(_cb._pV, Color());
}


void OwnerDrawButton::_DoPickClosed(const RECT& r)
{
	_r = r;
	_hPickColor = 0;
	Uncheck();
	assert(0 != _hWnd);
	InvalidateRect(_hWnd, 0, true);
}

