#include <cassert>

#include <string>

#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "window.h"
#include "keystate.h"
#include "sizestate.h"
#include "ownerdraw.h"
#include "ctrl.h"
#include "trackbarctrl.h"
#include "fwdefs.h"
#include "fwtmplts.h"
#include "canvas.h"
#include "formctrl.h"
#include "dialog.h"
#include "wndclass.h"
#include "gdiobjs.h"


#include "colorslider.h"
#include "colors.h"
#include "pickclrdlg.h"



void ColorSlider::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), WndProc);
	wc.style |= CS_GLOBALCLASS;
	wc.Register();
}


void ColorSlider::Unregister(HINSTANCE hInst)
{
	::UnregisterClass(_ClassName(), hInst);
}


LRESULT CALLBACK ColorSlider::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ColorSlider* pCtrl = GetWinLong<ColorSlider*>(hwnd);
	switch (msg)
	{
	case TBM_GETPOS :
		return pCtrl->GetPos();
	case TBM_SETRANGE :
		pCtrl->SetRange(wParam != 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case TBM_SETPOS :
		pCtrl->SetPos(wParam != 0, static_cast<int>(lParam));	
		return 0;
	default:
		return Wnd<ColorSlider>::Proc(hwnd, msg, wParam, lParam);
	}
}


ColorSlider::ColorSlider(HWND hwnd, const CREATESTRUCT* pCS) : 
Ctrl(hwnd, pCS),
_noButton(((pCS->style & CSS_NOCOLORBUTTON) != 0) || ((pCS->style & CSS_HUESLIDER) != 0)),
_hueSlider((pCS->style & CSS_HUESLIDER) != 0),
_hParent(pCS->hwndParent)
{
	_pCallback = 0;
	_pSharedPickColor = 0;
	_hPickColor = 0;
	_Position = -1;
	_Value = 0;
	_SlidingWidth = 1;
	_capturing = false;
	_Label.resize(0);

	_rangemin = 0;
	_rangemax = 100;
	_rangestep = 1;

	_minclr = RGB(0, 0, 0);

	if (_noButton)
		SetColor(255);
	else
		SetColor(RGB(255, 255, 255));

	_LastPickColorRect.left =
		_LastPickColorRect.right =
		_LastPickColorRect.bottom =
		_LastPickColorRect.top = 0;
}


ColorSlider::~ColorSlider()
{
	if (0 != _hPickColor)
	{
		if (0 == _pSharedPickColor)
			DestroyWindow(_hPickColor);
		_hPickColor = 0;
	}
}


void ColorSlider::SetColor(const float* arr)
{
	assert(arr[0]>=0.0f);
	assert(arr[0]<=1.0f);
	assert(arr[1]>=0.0f);
	assert(arr[1]<=1.0f);
	assert(arr[2]>=0.0f);
	assert(arr[2]<=1.0f);

	SetColor(RGB(static_cast<int>(arr[0]*255), static_cast<int>(arr[1]*255), static_cast<int>(arr[2]*255)));
}


void ColorSlider::SetColor(COLORREF clr)
{
	if (_noButton)
	{
		int v = clr;
		assert(v>=0);
		assert(v<256);
		clr = RGB(v, v, v);
	}
	int red = GetRValue(clr);
	int green = GetGValue(clr);
	int blue = GetBValue(clr);

	int maxval = red;
	if (green>maxval)
		maxval = green;
	if (blue>maxval)
		maxval = blue;

	// If input is black (0,0,0)
	if (0==maxval)
	{
		clr = RGB(255, 255, 255);
		_Position = 0;
		_Value = _rangemin;
	}
	else
	{
		float coeff = 255.0f/maxval;
		clr = RGB(static_cast<int>(red*coeff), static_cast<int>(green*coeff), static_cast<int>(blue*coeff));
		_Position = int((_SlidingWidth-1)/coeff);
		_Value = static_cast<int>(_Range()*(1.0f/coeff)+_rangemin);
	}

	_clr = clr;


	_RecalcGradientStep();
	if (0 != _hPickColor)
	{
		PickColorModeless* pPCM = GetWinLong<PickColorModeless*>(_hPickColor);
		pPCM->SetColor(clr);
	}
}

void ColorSlider::_RecalcGradientStep()
{
	if (_SlidingWidth>0)
	{
		_Rstep = (GetRValue(_clr)-GetRValue(_minclr))/static_cast<float>(_SlidingWidth);
		_Gstep = (GetGValue(_clr)-GetGValue(_minclr))/static_cast<float>(_SlidingWidth);
		_Bstep = (GetBValue(_clr)-GetBValue(_minclr))/static_cast<float>(_SlidingWidth);
	}
}

void ColorSlider::SetMaxColor(COLORREF clr, bool repaint)
{
	if (_clr != clr)
	{
		_clr = clr;
		_RecalcGradientStep();
		if (repaint)
			Invalidate();
	}
}


void ColorSlider::SetMinColor(COLORREF clr, bool repaint)
{
	if (_minclr != clr)
	{
		_minclr = clr;
		_RecalcGradientStep();
		if (repaint)
			Invalidate();
	}
}


bool ColorSlider::Size(SizeState, int, int)
{
	if (_noButton)
		_SlidingWidth = Width() - 3;
	else
		_SlidingWidth = Width() - Height() - 3;

	_RecalcGradientStep();
	if (_SlidingWidth>0)
		_rangestep = _rangemax/static_cast<float>(_SlidingWidth);

	float relv = 1.0f*(_Value-_rangemin)/_Range();
	_Position = static_cast<int>(_SlidingWidth * relv);
	return true;
}


void ColorSlider::ColorRangeRepaint(HDC hdc, int i) const
{
	const int from = (i-3 < 0) ? 0 : i-3;
	const int to = (i+3>=_SlidingWidth) ? _SlidingWidth : i+3;
	for (int j=from; j<to; j++)
	{
		COLORREF clr = 
			_hueSlider ? 
			_hsv[static_cast<int>(_hsvCount*static_cast<float>(j)/_SlidingWidth)]
			: 
		RGB(static_cast<int>(GetRValue(_minclr)+_Rstep*j), 
			static_cast<int>(GetGValue(_minclr)+_Gstep*j), 
			static_cast<int>(GetBValue(_minclr)+_Bstep*j));
		Pen pen(clr);
		ObjectHolder sp(hdc, pen);
		MoveToEx(hdc, j, 0, 0);
		LineTo(hdc, j, Height());
	}
	{
		ObjectHolder sp(hdc, pens3Dset.Face());
		for (int j=to; j<i+3; j++)
		{
			MoveToEx(hdc, j, 0, 0);
			LineTo(hdc, j, Height());
		}
	}
}

bool ColorSlider::Paint()
{
	PaintCanvas pc(Hwnd());

	// Color range
	if (_hueSlider)
	{
		if (_SlidingWidth>=_hsvCount)
		{
			int x0 = 0;
			int x1 = _SlidingWidth-1;
			int y0 = 0;
			int y1 = _hsvCount-1;
			assert(x1>=y1);
			int dx = x1-x0;
			int dy = y1-y0;
			int d = 2*dy-dx;
			int incE = 2*dy;
			int incNE = 2*(dy-dx);
			int x = x0;
			int y = y0;
			{
				Pen pen(_hsv[y]);
				ObjectHolder sp(pc, pen);
				pc.Line(x, 0, x, Height());
			}
			while (x<x1)
			{
				if (d<=0)
				{
					d += incE;
					x++;
				}
				else
				{
					d += incNE;
					x++;
					y++;
				}
				Pen pen(_hsv[y]);
				ObjectHolder sp(pc, pen);
				pc.Line(x, 0, x, Height());
			}
		}
		else
		{
			int x0 = 0;
			int x1 = _hsvCount-1;
			int y0 = 0;
			int y1 = _SlidingWidth-1;
			assert(x1>=y1);
			int dx = x1-x0;
			int dy = y1-y0;
			int d = 2*dy-dx;
			int incE = 2*dy;
			int incNE = 2*(dy-dx);
			int x = x0;
			int y = y0;
			{
				Pen pen(_hsv[x]);
				ObjectHolder sp(pc, pen);
				pc.Line(y, 0, y, Height());
			}
			while (x<x1)
			{
				if (d<=0)
				{
					d += incE;
					x++;
				}
				else
				{
					d += incNE;
					x++;
					y++;
					Pen pen(_hsv[x]);
					ObjectHolder sp(pc, pen);
					pc.Line(y, 0, y, Height());
				}
			}
		}
	}
	else
	{
		for (int i=0; i<_SlidingWidth; i++)
		{
			Pen pen(RGB
				(
				int(GetRValue(_minclr)+_Rstep*i), 
				int(GetGValue(_minclr)+_Gstep*i), 
				int(GetBValue(_minclr)+_Bstep*i)
				));
			ObjectHolder sp(pc, pen);
			pc.Line(i, 0, i, Height());
		}
	}
	{
		ObjectHolder sp(pc, pens3Dset.Face());
		for (int i=_SlidingWidth; i<Width(); i++)
			pc.Line(i, 0, i, Height());
	}

	if (!_noButton)
	{
		// Select color button
		const RECT buttonrect = { Width()-Height(), 0, Width(), Height() };
		// Interior
		{
			Brush br(_clr);
			ObjectHolder sp(pc, br);
			pc.FilledRectangle(buttonrect);
		}
		// Hilight
		{
			ObjectHolder sp
				(
				pc, 
				0 == _hPickColor ? pens3Dset.Hilight() : pens3Dset.Shadow()
				);
			pc.MoveTo(buttonrect.left, buttonrect.bottom);
			pc.LineTo(buttonrect.left, buttonrect.top);
			pc.LineTo(buttonrect.right, buttonrect.top);
		}
		// Light
		{
			ObjectHolder sp
				(
				pc, 
				0 == _hPickColor ? pens3Dset.Light() : pens3Dset.DkShadow()
				);
			pc.MoveTo(buttonrect.left+1, buttonrect.bottom-1);
			pc.LineTo(buttonrect.left+1, buttonrect.top+1);
			pc.LineTo(buttonrect.right-1, buttonrect.top+1);
		}
		// DkShadow
		{
			ObjectHolder sp
				(
				pc, 
				0 == _hPickColor ? pens3Dset.DkShadow() : pens3Dset.Light()
				);
			pc.MoveTo(buttonrect.left, buttonrect.bottom-1);
			pc.LineTo(buttonrect.right-1, buttonrect.bottom-1);
			pc.LineTo(buttonrect.right-1, buttonrect.top);
		}
		// Shadow
		{
			ObjectHolder sp
				(
				pc, 
				0 == _hPickColor ? pens3Dset.Shadow() : pens3Dset.Hilight()
				);
			pc.MoveTo(buttonrect.left+1, buttonrect.bottom-2);
			pc.LineTo(buttonrect.right-2, buttonrect.bottom-2);
			pc.LineTo(buttonrect.right-2, buttonrect.top+1);
		}

	}
	// The marker
	DrawMarker(pc);
	return true;
}

void ColorSlider::DrawMarker(Canvas& cnv)
{
	const int MarkerHeight = Height()/3;

	// Hilight
	{
		ObjectHolder sp(cnv, pens3Dset.Hilight());
		cnv.Line(_Position-2, Height(), _Position-2, MarkerHeight+2);
	}
	// Light
	{
		ObjectHolder sp(cnv, pens3Dset.Light());
		cnv.Line(_Position-1, Height(), _Position-1, MarkerHeight+1);
	}
	// Face
	{
		ObjectHolder sp(cnv, pens3Dset.Face());
		cnv.Line(_Position, Height(), _Position, MarkerHeight);
	}
	// Shadow
	{
		ObjectHolder sp(cnv, pens3Dset.Shadow());
		cnv.MoveTo(_Position-1, Height()-2);
		cnv.LineTo(_Position+1, Height()-2);
		cnv.LineTo(_Position+1, MarkerHeight+1);
	}
	// DkShadow
	{
		ObjectHolder sp(cnv, pens3Dset.DkShadow());
		cnv.MoveTo(_Position-1, Height()-1);
		cnv.LineTo(_Position+2, Height()-1);
		cnv.LineTo(_Position+2, MarkerHeight+2);
	}
}



bool ColorSlider::LButtonDown(KeyState, int x, int)
{
	if (x>=_SlidingWidth)
	{
		if (!_noButton)
			_ChangeColor();
	}
	else
	{
		SetCapture(Hwnd());
		UpdateCanvas cnv(Hwnd());
		_capturing = true;
		ColorRangeRepaint(cnv, _Position);
		_Position = x;
		_Value = _CalcValue();
		DrawMarker(cnv);
		if (0 != _pCallback)
		{
			if (_Position == _SlidingWidth-1)
				_pCallback->ColorChanged(_clr, true);
			else
				_pCallback->ColorChanged
				(
				RGB
				(
				static_cast<int>(GetRValue(_minclr)+_Position*_Rstep), 
				static_cast<int>(GetGValue(_minclr)+_Position*_Gstep), 
				static_cast<int>(GetBValue(_minclr)+_Position*_Bstep)
				), true
				);
		}
		if (0 != _hParent)
		{
			FORWARD_WM_HSCROLL(_hParent, Hwnd(), SB_THUMBTRACK, _Value, PostMessage);
		}
	}
	return true;
}

int ColorSlider::GetPos() const
{
	return _Value;
}

void ColorSlider::SetPos(bool repaint, int pos)
{
	if (pos<_rangemin)
		return;
	if (pos>_rangemax)
		return;
	if (pos != _Value)
	{
		_Value = pos;
		float relv = 1.0f*(_Value-_rangemin)/_Range();
		int oldpos = _Position;
		_Position = static_cast<int>(relv*_SlidingWidth);
		if (repaint)
		{
			UpdateCanvas cnv(Hwnd());
			ColorRangeRepaint(cnv, oldpos);
			DrawMarker(cnv);
		}
	}
}

void ColorSlider::SetRange(bool, int minval, int maxval)
{
	_rangemin = minval;
	_rangemax = maxval;
	if (_SlidingWidth>0)
		_rangestep = _rangemax/static_cast<float>(_SlidingWidth);
}

bool ColorSlider::LButtonUp(KeyState, int, int)
{
	if (_capturing)
	{
		_capturing = false;
		ReleaseCapture();
		if (0 != _pCallback)
		{
			if (_Position == _SlidingWidth-1)
				_pCallback->ColorChanged(_clr, true);
			else
				_pCallback->ColorChanged
				(
				RGB
				(
				static_cast<int>(GetRValue(_minclr)+_Position*_Rstep), 
				static_cast<int>(GetGValue(_minclr)+_Position*_Gstep), 
				static_cast<int>(GetBValue(_minclr)+_Position*_Bstep)
				), true
				);
		}
		if (0 != _hParent)
		{
			FORWARD_WM_HSCROLL(_hParent, Hwnd(), SB_THUMBPOSITION, _Value, PostMessage);
		}
	}
	return true;
}


bool ColorSlider::CaptureChanged()
{
	if (_capturing)
	{
		_capturing = false;
	}
	return true;
}


bool ColorSlider::MouseMove(KeyState, int x, int)
{
	if (_capturing)
	{
		if (x<0)
			x = 0;
		if (x>=_SlidingWidth)
			x = _SlidingWidth-1;

		if (x != _Position)
		{
			UpdateCanvas cnv(Hwnd());
			ColorRangeRepaint(cnv, _Position);
			_Position = x;
			_Value = _CalcValue();
			DrawMarker(cnv);
			if (0 != _pCallback)
			{
				if (_Position == _SlidingWidth-1)
					_pCallback->ColorChanged(_clr, false);
				else
					_pCallback->ColorChanged
					(
					RGB
					(
					static_cast<int>(GetRValue(_minclr)+_Position*_Rstep), 
					static_cast<int>(GetGValue(_minclr)+_Position*_Gstep), 
					static_cast<int>(GetBValue(_minclr)+_Position*_Bstep)
					), false
					);
			}
			if (0 != _hParent)
			{
				FORWARD_WM_HSCROLL(_hParent, Hwnd(), SB_THUMBTRACK, _Value, PostMessage);
			}
		}
	}
	return true;
}


void ColorSlider::_DoColorChanged(COLORREF clr)
{
	_clr = clr;
	_RecalcGradientStep();
	Invalidate();
	if (0 != _pCallback)
		_pCallback->ColorChanged
		(
		RGB
		(
		static_cast<int>(GetRValue(_minclr)+_Position*_Rstep), 
		static_cast<int>(GetGValue(_minclr)+_Position*_Gstep), 
		static_cast<int>(GetBValue(_minclr)+_Position*_Bstep)
		), false
		);
}


void ColorSlider::_ChangeColor()
{
	if (0 != _pSharedPickColor)
	{
		if (_pSharedPickColor->CurrentOwner() == this)
		{
			_pSharedPickColor->Close();
			_hPickColor = 0;
		}
		else
		{
			_hPickColor = _pSharedPickColor->SetCurrentOwner(this);
		}
		Invalidate();
	}
	else if (0 != _hPickColor)
	{
		SetForegroundWindow(_hPickColor);
	}
	else
	{
		PickColorModeless::Callbacks cb(_ColorChangedClbck, _PickClosedClbck, this);
		PickColorModeless::InitData init(&cb, _Label.c_str(), _clr, _LastPickColorRect);
		_hPickColor = 
			PickColorModeless::Create(Hwnd(), &init);
		Invalidate();
	}
}




const COLORREF ColorSlider::_hsv[] =
{
	RGB(255,0,0),RGB(255,0,0),RGB(255,4,0),RGB(255,8,0),
	RGB(255,12,0),RGB(255,17,0),RGB(255,21,0),RGB(255,25,0),
	RGB(255,29,0),RGB(255,34,0),RGB(255,38,0),RGB(255,42,0),
	RGB(255,46,0),RGB(255,51,0),RGB(255,55,0),RGB(255,59,0),
	RGB(255,63,0),RGB(255,68,0),RGB(255,72,0),RGB(255,76,0),
	RGB(255,80,0),RGB(255,85,0),RGB(255,89,0),RGB(255,93,0),
	RGB(255,97,0),RGB(255,102,0),RGB(255,106,0),RGB(255,110,0),
	RGB(255,114,0),RGB(255,119,0),RGB(255,123,0),RGB(255,127,0),
	RGB(255,131,0),RGB(255,136,0),RGB(255,140,0),RGB(255,144,0),
	RGB(255,148,0),RGB(255,153,0),RGB(255,157,0),RGB(255,161,0),
	RGB(255,165,0),RGB(255,170,0),RGB(255,174,0),RGB(255,178,0),
	RGB(255,182,0),RGB(255,187,0),RGB(255,191,0),RGB(255,195,0),
	RGB(255,199,0),RGB(255,204,0),RGB(255,208,0),RGB(255,212,0),
	RGB(255,216,0),RGB(255,221,0),RGB(255,225,0),RGB(255,229,0),
	RGB(255,233,0),RGB(255,238,0),RGB(255,242,0),RGB(255,246,0),
	RGB(255,250,0),RGB(255,255,0),RGB(250,255,0),RGB(246,255,0),
	RGB(242,255,0),RGB(238,255,0),RGB(233,255,0),RGB(229,255,0),
	RGB(225,255,0),RGB(221,255,0),RGB(216,255,0),RGB(212,255,0),
	RGB(208,255,0),RGB(204,255,0),RGB(199,255,0),RGB(195,255,0),
	RGB(191,255,0),RGB(187,255,0),RGB(182,255,0),RGB(178,255,0),
	RGB(174,255,0),RGB(170,255,0),RGB(165,255,0),RGB(161,255,0),
	RGB(157,255,0),RGB(153,255,0),RGB(148,255,0),RGB(144,255,0),
	RGB(140,255,0),RGB(136,255,0),RGB(131,255,0),RGB(127,255,0),
	RGB(123,255,0),RGB(118,255,0),RGB(114,255,0),RGB(110,255,0),
	RGB(106,255,0),RGB(101,255,0),RGB(97,255,0),RGB(93,255,0),
	RGB(89,255,0),RGB(84,255,0),RGB(80,255,0),RGB(76,255,0),
	RGB(72,255,0),RGB(67,255,0),RGB(63,255,0),RGB(59,255,0),
	RGB(55,255,0),RGB(50,255,0),RGB(46,255,0),RGB(42,255,0),
	RGB(38,255,0),RGB(33,255,0),RGB(29,255,0),RGB(25,255,0),
	RGB(21,255,0),RGB(16,255,0),RGB(12,255,0),RGB(8,255,0),
	RGB(4,255,0),RGB(0,255,0),RGB(0,255,4),RGB(0,255,8),
	RGB(0,255,12),RGB(0,255,17),RGB(0,255,21),RGB(0,255,25),
	RGB(0,255,29),RGB(0,255,33),RGB(0,255,38),RGB(0,255,42),
	RGB(0,255,46),RGB(0,255,51),RGB(0,255,55),RGB(0,255,59),
	RGB(0,255,63),RGB(0,255,67),RGB(0,255,72),RGB(0,255,76),
	RGB(0,255,80),RGB(0,255,85),RGB(0,255,89),RGB(0,255,93),
	RGB(0,255,97),RGB(0,255,101),RGB(0,255,106),RGB(0,255,110),
	RGB(0,255,114),RGB(0,255,119),RGB(0,255,123),RGB(0,255,127),
	RGB(0,255,131),RGB(0,255,135),RGB(0,255,140),RGB(0,255,144),
	RGB(0,255,148),RGB(0,255,153),RGB(0,255,157),RGB(0,255,161),
	RGB(0,255,165),RGB(0,255,169),RGB(0,255,174),RGB(0,255,178),
	RGB(0,255,182),RGB(0,255,187),RGB(0,255,191),RGB(0,255,195),
	RGB(0,255,199),RGB(0,255,203),RGB(0,255,208),RGB(0,255,212),
	RGB(0,255,216),RGB(0,255,221),RGB(0,255,225),RGB(0,255,229),
	RGB(0,255,233),RGB(0,255,237),RGB(0,255,242),RGB(0,255,246),
	RGB(0,255,250),RGB(0,255,255),RGB(0,250,255),RGB(0,246,255),
	RGB(0,242,255),RGB(0,237,255),RGB(0,233,255),RGB(0,229,255),
	RGB(0,225,255),RGB(0,221,255),RGB(0,216,255),RGB(0,212,255),
	RGB(0,208,255),RGB(0,203,255),RGB(0,199,255),RGB(0,195,255),
	RGB(0,191,255),RGB(0,187,255),RGB(0,182,255),RGB(0,178,255),
	RGB(0,174,255),RGB(0,169,255),RGB(0,165,255),RGB(0,161,255),
	RGB(0,157,255),RGB(0,153,255),RGB(0,148,255),RGB(0,144,255),
	RGB(0,140,255),RGB(0,135,255),RGB(0,131,255),RGB(0,127,255),
	RGB(0,123,255),RGB(0,119,255),RGB(0,114,255),RGB(0,110,255),
	RGB(0,106,255),RGB(0,101,255),RGB(0,97,255),RGB(0,93,255),
	RGB(0,89,255),RGB(0,85,255),RGB(0,80,255),RGB(0,76,255),
	RGB(0,72,255),RGB(0,67,255),RGB(0,63,255),RGB(0,59,255),
	RGB(0,55,255),RGB(0,51,255),RGB(0,46,255),RGB(0,42,255),
	RGB(0,38,255),RGB(0,33,255),RGB(0,29,255),RGB(0,25,255),
	RGB(0,21,255),RGB(0,17,255),RGB(0,12,255),RGB(0,8,255),
	RGB(0,4,255),RGB(0,0,255),RGB(4,0,255),RGB(8,0,255),
	RGB(12,0,255),RGB(16,0,255),RGB(21,0,255),RGB(25,0,255),
	RGB(29,0,255),RGB(34,0,255),RGB(38,0,255),RGB(42,0,255),
	RGB(46,0,255),RGB(51,0,255),RGB(55,0,255),RGB(59,0,255),
	RGB(63,0,255),RGB(67,0,255),RGB(72,0,255),RGB(76,0,255),
	RGB(80,0,255),RGB(84,0,255),RGB(89,0,255),RGB(93,0,255),
	RGB(97,0,255),RGB(102,0,255),RGB(106,0,255),RGB(110,0,255),
	RGB(114,0,255),RGB(119,0,255),RGB(123,0,255),RGB(127,0,255),
	RGB(131,0,255),RGB(135,0,255),RGB(140,0,255),RGB(144,0,255),
	RGB(148,0,255),RGB(152,0,255),RGB(157,0,255),RGB(161,0,255),
	RGB(165,0,255),RGB(170,0,255),RGB(174,0,255),RGB(178,0,255),
	RGB(182,0,255),RGB(187,0,255),RGB(191,0,255),RGB(195,0,255),
	RGB(199,0,255),RGB(203,0,255),RGB(208,0,255),RGB(212,0,255),
	RGB(216,0,255),RGB(220,0,255),RGB(225,0,255),RGB(229,0,255),
	RGB(233,0,255),RGB(238,0,255),RGB(242,0,255),RGB(246,0,255),
	RGB(250,0,255),RGB(255,0,255),RGB(255,0,250),RGB(255,0,246),
	RGB(255,0,242),RGB(255,0,238),RGB(255,0,233),RGB(255,0,229),
	RGB(255,0,225),RGB(255,0,220),RGB(255,0,216),RGB(255,0,212),
	RGB(255,0,208),RGB(255,0,203),RGB(255,0,199),RGB(255,0,195),
	RGB(255,0,191),RGB(255,0,187),RGB(255,0,182),RGB(255,0,178),
	RGB(255,0,174),RGB(255,0,170),RGB(255,0,165),RGB(255,0,161),
	RGB(255,0,157),RGB(255,0,152),RGB(255,0,148),RGB(255,0,144),
	RGB(255,0,140),RGB(255,0,135),RGB(255,0,131),RGB(255,0,127),
	RGB(255,0,123),RGB(255,0,119),RGB(255,0,114),RGB(255,0,110),
	RGB(255,0,106),RGB(255,0,102),RGB(255,0,97),RGB(255,0,93),
	RGB(255,0,89),RGB(255,0,84),RGB(255,0,80),RGB(255,0,76),
	RGB(255,0,72),RGB(255,0,67),RGB(255,0,63),RGB(255,0,59),
	RGB(255,0,55),RGB(255,0,51),RGB(255,0,46),RGB(255,0,42),
	RGB(255,0,38),RGB(255,0,34),RGB(255,0,29),RGB(255,0,25),
	RGB(255,0,21),RGB(255,0,16),RGB(255,0,12),RGB(255,0,8)
};


const int ColorSlider::_hsvCount = CountOf(ColorSlider::_hsv);



HWND SharedPickColor::SetCurrentOwner(ColorSlider* pSlider)
{
	if (0 != _pSlider)
		_pSlider->_DoPickClosed(_rect);

	_pSlider = pSlider;
	if (0 == _hPickColor)
	{
		PickColorModeless::Callbacks cb(_ColorChangedClbck, _PickClosedClbck, this);
		PickColorModeless::InitData init(&cb, _pSlider->_Label.c_str(), _pSlider->_clr, _rect);
		_hPickColor = 
			PickColorModeless::Create(_pSlider->Hwnd(), &init);
	}
	else
	{
		PickColorModeless* pPCM = GetWinLong<PickColorModeless*>(_hPickColor);
		pPCM->Reassign(_pSlider->_Label.c_str(), _pSlider->_clr);
	}
	return _hPickColor;
}

ColorSlider* SharedPickColor::CurrentOwner() const
{ return _pSlider; }

void SharedPickColor::Close()
{
	if (0 != _hPickColor)
		DestroyWindow(_hPickColor);
}

SharedPickColor::SharedPickColor()
{
	_rect.left = _rect.right = _rect.bottom = _rect.top = 0;
	_hPickColor = 0;
	_pSlider = 0;
}

SharedPickColor::~SharedPickColor()
{
	if (0 != _hPickColor)
		DestroyWindow(_hPickColor);
}

void SharedPickColor::_DoColorChanged(COLORREF clr)
{
	assert(0 != _pSlider);
	_pSlider->_DoColorChanged(clr);
}


void SharedPickColor::_DoPickClosed(const RECT& r)
{
	_rect = r;
	_pSlider->_DoPickClosed(r);
	_hPickColor = 0;
	_pSlider = 0;
}

void SharedPickColor::Show(bool show)
{
	if (_hPickColor != 0)
	{
		if (show)
			ShowWindow(_hPickColor, SW_SHOWNOACTIVATE);
		else
			ShowWindow(_hPickColor, SW_HIDE);
	}
}
