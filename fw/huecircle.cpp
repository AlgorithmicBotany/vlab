#include <cassert>
#include <cmath>
#include <string>

#include <windows.h>
#include <windowsx.h>

#include "warningset.h"

#include "colors.h"
#include "exception.h"
#include "menu.h"
#include "window.h"
#include "keystate.h"
#include "sizestate.h"
#include "ownerdraw.h"
#include "ctrl.h"
#include "canvas.h"
#include "huecircle.h"
#include "wndclass.h"
#include "fwdefs.h"
#include "fwtmplts.h"
#include "bitmap.h"


HBITMAP HueCircle::_hHue0128 = 0;
HBITMAP HueCircle::_hHue0256 = 0;
HBITMAP HueCircle::_hHue0512 = 0;


void HueCircle::Register(HINSTANCE hInst)
{
	WndClass wnd(hInst, _ClassName(), Wnd<HueCircle>::Proc);
	wnd.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
	wnd.Register();
	_hHue0128 = LoadBitmap(hInst, "HUE0128");
	_hHue0256 = LoadBitmap(hInst, "HUE0256");
	_hHue0512 = LoadBitmap(hInst, "HUE0512");
}

void HueCircle::Unregister(HINSTANCE hInst)
{
	::UnregisterClass(_ClassName(), hInst);
	::DeleteObject(_hHue0128); _hHue0128 = 0;
	::DeleteObject(_hHue0256); _hHue0256 = 0;
	::DeleteObject(_hHue0512); _hHue0512 = 0;
}

HueCircle::HueCircle(HWND hWnd, const CREATESTRUCT* pCS) : 
Ctrl(hWnd, pCS)
{
	_selection.x = 0;
	_selection.y = 0;
	_r = 0;
	_dragging = false;
	_Hue = 0.0f;
	_Saturation = 0.0;
	_pClbck = 0;
	_pClbckParam = 0;
	_hBmp = 0;
}


HueCircle::~HueCircle()
{}


bool HueCircle::Paint()
{
	PaintCanvas pc(Hwnd());

	{
		Bitmap bmp(_hBmp);
		bmp.Draw(pc, _lt.x, _lt.y, _bmpsize, _bmpsize);
		bmp.Release();
	}

	SetROP2(pc, R2_NOT);
	_DrawIndicator(pc);
	return true;
}



bool HueCircle::Size(SizeState, int w, int h)
{
	int mindim = w;
	if (h<w)
		mindim = h;
	mindim -= 2;
	_center.x = w/2;
	_center.y = h/2;
	_bmpsize = mindim;
	if (_bmpsize<256)
	{
		_hBmp = _hHue0128;
		_bmpsize = 128;
	}
	else if (_bmpsize<512)
	{
		_hBmp = _hHue0256;
		_bmpsize = 256;
	}
	else
	{
		_hBmp = _hHue0512;
		_bmpsize = 512;
	}
	_lt.x = _center.x-_bmpsize/2;
	_lt.y = _center.y-_bmpsize/2;
	return true;
}


bool HueCircle::LButtonDown(KeyState, int x, int y)
{
	GrabFocus();
	UpdateCanvas cnv(Hwnd());
	SetROP2(cnv, R2_NOT);
	_DrawIndicator(cnv);
	_DragTo(x, y);
	_DrawIndicator(cnv);
	if (0 != _pClbck)
		_pClbck(_Hue, _Saturation, _pClbckParam);
	_dragging = true;
	SetCapture(Hwnd());
	return true;
}


bool HueCircle::MouseMove(KeyState, int x, int y)
{
	if (_dragging)
	{
		UpdateCanvas cnv(Hwnd());
		SetROP2(cnv, R2_NOT);
		_DrawIndicator(cnv);
		_DragTo(x, y);
		_DrawIndicator(cnv);
		if (0 != _pClbck)
			_pClbck(_Hue, _Saturation, _pClbckParam);
	}
	return true;
}


bool HueCircle::LButtonUp(KeyState, int, int)
{
	if (_dragging)
		ReleaseCapture();
	return true;
}


bool HueCircle::CaptureChanged()
{
	assert(_dragging);
	_dragging = false;
	return true;
}


void HueCircle::_DragTo(int x, int y)
{
	int newx = x-_center.x;
	int newy = y-_center.y;
	if (newx==0 && newy==0)
	{
		_Saturation = 0.0;
		_selection.x = newx;
		_selection.y = newy;
		_r = 0;
	}
	else
	{
		// reverse y as screen coordinates go downwards
		// while for the geometrical calculations we need
		// them the other way around
		float nx = static_cast<float>(newx);
		float ny = static_cast<float>(-newy);
		float r = sqrtf(nx*nx+ny*ny);
		float huedeg = -1.0f;
		if (newx>=0 && -newy>=0)
		{
			float acs = nx/r;
			huedeg = acosf(acs)*180.0f/M_PIf;
			assert(huedeg>=0.0f);
			assert(huedeg<=90.0f);
		}
		else if (newx<0 && -newy>=0)
		{
			float acs = nx/r;
			huedeg = acosf(acs)*180.0f/M_PIf;
			assert(huedeg>90.0f);
			assert(huedeg<=180.0f);
		}
		else if (newx<0 /* && -newy<0 */)
		{
			float acs = nx/r;
			huedeg = 360.0f-acosf(acs)*180.0f/M_PIf;
			assert(huedeg>180.0f);
			assert(huedeg<270.0f);
		}
		else /* newx>=0 && -newy>=0 */
		{
			float acs = nx/r;
			huedeg = 360.0f-acosf(acs)*180.0f/M_PIf;
			assert(huedeg>=270.0f);
			assert(huedeg<360.0f);
		}
		_Hue = huedeg/360.0f;
		assert(_Hue>=0.0f);
		assert(_Hue<1.0f);
		_Saturation = r/(_bmpsize/2);
		if (_Saturation>1.0f)
		{
			nx /= _Saturation;
			ny /= _Saturation;
			newx = static_cast<int>(floorf(nx+0.5f));
			newy = -static_cast<int>(floorf(ny+0.5f));
			_Saturation = 1.0;
			_r = _bmpsize/2;
		}
		else
			_r = static_cast<int>(sqrtf(nx*nx+ny*ny)+0.5f);
		_selection.x = newx;
		_selection.y = newy;
	}
}



int HueCircle::GetDlgCode()
{
	return DLGC_WANTARROWS;
}


bool HueCircle::KeyDown(UINT vk)
{
	switch (vk)
	{
	case VK_UP :
		{
			LButtonDown(KeyState(0),_selection.x+_center.x, _selection.y-1+_center.y);
			LButtonUp(KeyState(0), 0, 0);
		}
		break;
	case VK_DOWN :
		{
			LButtonDown(KeyState(0), _selection.x+_center.x, _selection.y+1+_center.y);
			LButtonUp(KeyState(0), 0, 0);
		}
		break;
	case VK_LEFT :
		{
			LButtonDown(KeyState(0), _selection.x-1+_center.x, _selection.y+_center.y);
			LButtonUp(KeyState(0), 0, 0);
		}
		break;
	case VK_RIGHT :
		{
			LButtonDown(KeyState(0), _selection.x+1+_center.x, _selection.y+_center.y);
			LButtonUp(KeyState(0), 0, 0);
		}
		break;
	}
	return true;
}


void HueCircle::SetColor(ColorHSVf hsv)
{
	UpdateCanvas cnv(Hwnd());
	SetROP2(cnv, R2_NOT);
	_DrawIndicator(cnv);
	_Hue = hsv.H()/360.0f;
	_Saturation = hsv.S();
	_selection.x = static_cast<int>(_Saturation* (_bmpsize/2) * cosf(2.0f*M_PIf*_Hue));
	_selection.y = -static_cast<int>(_Saturation* (_bmpsize/2) * sinf(2.0f*M_PIf*_Hue));
	{
		const float dx = static_cast<float>(_selection.x);
		const float dy = static_cast<float>(_selection.y);
		_r = static_cast<int>(sqrtf(dx*dx + dy*dy)+0.5f);
	}
	_DrawIndicator(cnv);
}
