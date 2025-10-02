#include <cassert>
#include <cmath>

#include <string>
#include <locale>

#include <windows.h>
#include <tchar.h>
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
#include "canvas.h"
#include "colors.h"
#include "dialog.h"
#include "formctrl.h"
#include "fwdefs.h"
#include "fwtmplts.h"
#include "pickclrdlg.h"
#include "huecircle.h"
#include "colorslider.h"
#include "mdimenus.h"
#include "app.h"
#include "fwres.h"



static const TCHAR Fmt[] = __TEXT("%3.2f");

PickColorBase::PickColorBase(COLORREF clr, const Callbacks* pCB) :
_rgb(clr), _hsv(clr),
_HueSlider(Window(0)), _SatSlider(Window(0)), _ValSlider(Window(0)),
_RedSlider(Window(0)), _GreenSlider(Window(0)), _BlueSlider(Window(0))
{
	_pCirc = 0;
	if (0 != pCB)
		_cb = *pCB;
	_pValSlider = 0;
	_pSatSlider = 0;
	_hIcon = 0;
}

PickColorBase::~PickColorBase()
{
	if (0 != _hIcon)
		DestroyIcon(_hIcon);
}

const PickColorBase::Callbacks cb(0, 0, 0);

void PickColorBase::SetColor(COLORREF clr)
{
	_rgb.Set(clr);
	_hsv.Set(_rgb);
	_UpdateEdits();
	_UpdateSliders();
	_pCirc->SetColor(_hsv);
	_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
	_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
}

PickColorDlg::PickColorDlg(COLORREF clr) : 
Dialog("PICKCOLORMODAL", App::GetInstance()),
PickColorBase(clr, &cb)
{
}


bool PickColorDlg::DoInit()
{
	_Init(Hdlg(), HInstance());
	return true;
}


void PickColorBase::_Init(HWND hDlg, HINSTANCE hInst)
{
	if (0==_hIcon)
		_hIcon = (HICON)LoadImage(hInst, "HSCIRCLE", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM) _hIcon);

	TCHAR bf[32];
	_hHue = GetDlgItem(hDlg,IDC_HUE);
	assert(_hHue != 0);
	_stprintf(bf, Fmt, _hsv.H()/360.0f);
	SetWindowText(_hHue, bf);

	_HueSlider.SetHwnd(GetDlgItem(hDlg,IDC_HUESLIDER));
	_HueSlider.SetRange(0, 100);
	_HueSlider.SetValue(static_cast<int>(_hsv.H()/3.6f));

	_hSat = GetDlgItem(hDlg,IDC_SATURATION);
	assert(_hSat != 0);
	_stprintf(bf, Fmt, _hsv.S());
	SetWindowText(_hSat, bf);

	_SatSlider.SetHwnd(GetDlgItem(hDlg,IDC_SATURATIONSLIDER));
	_SatSlider.SetRange(0, 100);
	_SatSlider.SetValue(static_cast<int>(_hsv.S()*100.0f));
	{
		_pSatSlider = GetWinLong<ColorSlider*>(_SatSlider.Hwnd());
		_pSatSlider->SetMaxColor(_hsv.GetHueInRGB());
		_pSatSlider->SetMinColor(RGB(255, 255, 255));
	}

	_hVal = GetDlgItem(hDlg,IDC_VALUE);
	assert(_hVal != 0);
	_stprintf(bf, Fmt, _hsv.V());
	SetWindowText(_hVal, bf);

	_ValSlider.SetHwnd(GetDlgItem(hDlg,IDC_VALUESLIDER));
	_ValSlider.SetRange(0, 100);
	_ValSlider.SetValue(static_cast<int>(_hsv.V()*100.0f));
	{
		_pValSlider = GetWinLong<ColorSlider*>(_ValSlider.Hwnd());
		_pValSlider->SetMaxColor(_hsv.GetHueInRGB());
	}

	_hRed = GetDlgItem(hDlg,IDC_RED);
	assert(_hRed != 0);
	_stprintf(bf, Fmt, _rgb.R());
	SetWindowText(_hRed, bf);

	_RedSlider.SetHwnd(GetDlgItem(hDlg,IDC_REDSLIDER));
	_RedSlider.SetRange(0, 100);
	_RedSlider.SetValue(static_cast<int>(_rgb.R()*100.0f));
	{
		ColorSlider* pSlider = GetWinLong<ColorSlider*>(_RedSlider.Hwnd());
		pSlider->SetMaxColor(RGB(255, 0, 0));
	}

	_hGreen = GetDlgItem(hDlg,IDC_GREEN);
	assert(_hGreen != 0);
	_stprintf(bf, Fmt, _rgb.G());
	SetWindowText(_hGreen, bf);

	_GreenSlider.SetHwnd(GetDlgItem(hDlg,IDC_GREENSLIDER));
	_GreenSlider.SetRange(0, 100);
	_GreenSlider.SetValue(static_cast<int>(_rgb.G()*100.0f));
	{
		ColorSlider* pSlider = GetWinLong<ColorSlider*>(_GreenSlider.Hwnd());
		pSlider->SetMaxColor(RGB(0, 255, 0));
	}

	_hBlue = GetDlgItem(hDlg,IDC_BLUE);
	assert(_hBlue != 0);
	_stprintf(bf, Fmt, _rgb.B());
	SetWindowText(_hBlue, bf);

	_BlueSlider.SetHwnd(GetDlgItem(hDlg,IDC_BLUESLIDER));
	_BlueSlider.SetRange(0, 100);
	_BlueSlider.SetValue(static_cast<int>(_rgb.B()*100.0f));
	{
		ColorSlider* pSlider = GetWinLong<ColorSlider*>(_BlueSlider.Hwnd());
		pSlider->SetMaxColor(RGB(0, 0, 255));
	}

	_pCirc = GetWinLong<HueCircle*>(GetDlgItem(hDlg,IDC_HUECIRCLE));
	_pCirc->SetNotifyCallback(PickColorBase::HueSatCallback, this);
	_pCirc->SetColor(_hsv);
}


void PickColorBase::HueSatCallback(float hue, float sat, void* pV)
{
	PickColorBase* pCtrl = reinterpret_cast<PickColorBase*>(pV);
	pCtrl->DoHueSat(hue, sat);
}


void PickColorBase::DoHueSat(float hue, float sat)
{
	_hsv.H(hue*360.0f);
	_hsv.S(sat);
	_rgb.Set(_hsv);
	_UpdateSliders();
	_UpdateEdits();
	_Modified();
	_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
	_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
}


void PickColorBase::_UpdateSliders()
{
	_UpdateRGBSliders();
	_UpdateHSVSliders();
}

void PickColorBase::_UpdateHSVSliders()
{
	_HueSlider.SetValue(static_cast<int>(_hsv.H()/3.6f));
	_SatSlider.SetValue(static_cast<int>(_hsv.S()*100.0f));
	_ValSlider.SetValue(static_cast<int>(_hsv.V()*100.0f));
}

void PickColorBase::_UpdateRGBSliders()
{
	_RedSlider.SetValue(static_cast<int>(_rgb.R()*100.0f));
	_GreenSlider.SetValue(static_cast<int>(_rgb.G()*100.0f));
	_BlueSlider.SetValue(static_cast<int>(_rgb.B()*100.0f));
}


void PickColorBase::_UpdateEdits()
{
	_UpdateHSVEdits();
	_UpdateRGBEdits();
}

void PickColorBase::_UpdateHSVEdits()
{
	TCHAR bf[32];
	_stprintf(bf, Fmt, _hsv.H()/360.0f);
	SetWindowText(_hHue, bf);
	_stprintf(bf, Fmt, _hsv.S());
	SetWindowText(_hSat, bf);
	_stprintf(bf, Fmt, _hsv.V());
	SetWindowText(_hVal, bf);
}

void PickColorBase::_UpdateRGBEdits()
{
	TCHAR bf[32];
	_stprintf(bf, Fmt, _rgb.R());
	SetWindowText(_hRed, bf);
	_stprintf(bf, Fmt, _rgb.G());
	SetWindowText(_hGreen, bf);
	_stprintf(bf, Fmt, _rgb.B());
	SetWindowText(_hBlue, bf);
}



bool PickColorDlg::HandleMsg(HWND hWnd, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg)
	{
	case WM_HSCROLL :
		PCBHScroll(hWnd, reinterpret_cast<HWND>(lParam), 0, 0);
		break;
	default :
		return false;
	}
	return true;
}


void PickColorBase::PCBHScroll(HWND, HWND hCtl, UINT, int)
{
	TCHAR bf[32];
	if (hCtl == _HueSlider.Hwnd())
	{
		int v = _HueSlider.GetValue();
		_stprintf(bf, Fmt, 0.01f*v);
		SetWindowText(_hHue, bf);

		if (100==v)
			v = 0;
		_hsv.H(3.6f*v);
		_rgb.Set(_hsv);
		_UpdateRGBSliders();
		_UpdateRGBEdits();
		_pCirc->SetColor(_hsv);
		_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
		_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);

		_Modified();
	}
	else if (hCtl == _SatSlider.Hwnd())
	{
		int v = _SatSlider.GetValue();
		_stprintf(bf, Fmt, 0.01f*v);
		SetWindowText(_hSat, bf);

		_hsv.S(0.01f*v);
		_rgb.Set(_hsv);
		_UpdateRGBSliders();
		_UpdateRGBEdits();
		_pCirc->SetColor(_hsv);

		_Modified();
	}
	else if (hCtl == _ValSlider.Hwnd())
	{
		int v = _ValSlider.GetValue();
		_stprintf(bf, Fmt, 0.01f*v);
		SetWindowText(_hVal, bf);

		_hsv.V(0.01f*v);
		_rgb.Set(_hsv);
		_UpdateRGBSliders();
		_UpdateRGBEdits();

		_Modified();
	}
	else if (hCtl == _RedSlider.Hwnd())
	{
		int v = _RedSlider.GetValue();
		_stprintf(bf, Fmt, 0.01f*v);
		SetWindowText(_hRed, bf);

		_rgb.R(0.01f*v);
		_hsv.Set(_rgb);
		_UpdateHSVSliders();
		_UpdateHSVEdits();
		_pCirc->SetColor(_hsv);
		_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
		_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);

		_Modified();
	}
	else if (hCtl == _GreenSlider.Hwnd())
	{
		int v = _GreenSlider.GetValue();
		_stprintf(bf, Fmt, 0.01f*v);
		SetWindowText(_hGreen, bf);

		_rgb.G(0.01f*v);
		_hsv.Set(_rgb);
		_UpdateHSVSliders();
		_UpdateHSVEdits();
		_pCirc->SetColor(_hsv);
		_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
		_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);

		_Modified();
	}
	else if (hCtl == _BlueSlider.Hwnd())
	{
		int v = _BlueSlider.GetValue();
		_stprintf(bf, Fmt, 0.01f*v);
		SetWindowText(_hBlue, bf);

		_rgb.B(0.01f*v);
		_hsv.Set(_rgb);
		_UpdateHSVSliders();
		_UpdateHSVEdits();
		_pCirc->SetColor(_hsv);
		_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
		_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);

		_Modified();
	}
}


void PickColorBase::_Modified()
{
	if (0 != _cb._pChng)
		(_cb._pChng)(_cb._pV, _rgb.Get());
}

bool PickColorDlg::Command(int id, Window, UINT ncode)
{
	switch (id)
	{
	case IDC_APPLY :
		_Apply();
		break;
	case IDC_RED :
		if (EN_KILLFOCUS == ncode)
			_RChanged();
		break;
	case IDC_GREEN :
		if (EN_KILLFOCUS == ncode)
			_GChanged();
		break;
	case IDC_BLUE :
		if (EN_KILLFOCUS == ncode)
			_BChanged();
		break;
	case IDC_HUE :
		if (EN_KILLFOCUS == ncode)
			_HChanged();
		break;
	case IDC_SATURATION :
		if (EN_KILLFOCUS == ncode)
			_SChanged();
		break;
	case IDC_VALUE :
		if (EN_KILLFOCUS == ncode)
			_VChanged();
		break;
	default :
		return false;
	}
	return true;
}


void PickColorBase::_RChanged()
{
	float v;
	if (_GetEditFloat(_hRed, v))
	{
		_RedSlider.SetValue(static_cast<int>(v*100.0f));
		_rgb.R(v);
		_hsv.Set(_rgb);
		_UpdateHSVSliders();
		_UpdateHSVEdits();
		_pCirc->SetColor(_hsv);
		_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
		_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
		_Modified();
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		int v = _RedSlider.GetValue();
		TCHAR bf[32];
		_stprintf(bf, Fmt, 0.01*v);
		SetWindowText(_hRed, bf);
	}
}


void PickColorBase::_GChanged()
{
	float v;
	if (_GetEditFloat(_hGreen, v))
	{
		_GreenSlider.SetValue(static_cast<int>(v*100.0f));
		_rgb.G(v);
		_hsv.Set(_rgb);
		_UpdateHSVSliders();
		_UpdateHSVEdits();
		_pCirc->SetColor(_hsv);
		_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
		_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
		_Modified();
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		int v = _GreenSlider.GetValue();
		TCHAR bf[32];
		_stprintf(bf, Fmt, 0.01f*v);
		SetWindowText(_hGreen, bf);
	}
}

void PickColorBase::_BChanged()
{
	float v;
	if (_GetEditFloat(_hBlue, v))
	{
		_BlueSlider.SetValue(static_cast<int>(v*100.0f));
		_rgb.B(v);
		_hsv.Set(_rgb);
		_UpdateHSVSliders();
		_UpdateHSVEdits();
		_pCirc->SetColor(_hsv);
		_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
		_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
		_Modified();
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		int v = _BlueSlider.GetValue();
		TCHAR bf[32];
		_stprintf(bf, Fmt, 0.01f*v);
		SetWindowText(_hBlue, bf);
	}
}

void PickColorBase::_HChanged()
{
	float v;
	if (_GetEditFloat(_hHue, v))
	{
		_HueSlider.SetValue(static_cast<int>(v*100.0f));
		if (1.0f==v)
			v = 0.0f;
		_hsv.H(360.0f*v);
		_rgb.Set(_hsv);
		_UpdateRGBSliders();
		_UpdateRGBEdits();
		_pCirc->SetColor(_hsv);
		_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
		_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
		_Modified();
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		int v = _HueSlider.GetValue();
		TCHAR bf[32];
		_stprintf(bf, Fmt, 0.01f*v);
		SetWindowText(_hHue, bf);
	}
}


void PickColorBase::_SChanged()
{
	float v;
	if (_GetEditFloat(_hSat, v))
	{
		_SatSlider.SetValue(static_cast<int>(v*100.0f));
		if (1.0f==v)
			v = 0.0f;
		_hsv.S(v);
		_rgb.Set(_hsv);
		_UpdateRGBSliders();
		_UpdateRGBEdits();
		_pCirc->SetColor(_hsv);
		_Modified();
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		int v = _SatSlider.GetValue();
		TCHAR bf[32];
		_stprintf(bf, Fmt, 0.01f*v);
		SetWindowText(_hSat, bf);
	}
}


void PickColorBase::_VChanged()
{
	float v;
	if (_GetEditFloat(_hVal, v))
	{
		_ValSlider.SetValue(static_cast<int>(v*100.0f));
		if (1.0f==v)
			v = 0.0f;
		_hsv.V(v);
		_rgb.Set(_hsv);
		_UpdateRGBSliders();
		_UpdateRGBEdits();
		_pCirc->SetColor(_hsv);
		_Modified();
	}
	else
	{
		MessageBeep(0xFFFFFFFF);
		int v = _ValSlider.GetValue();
		TCHAR bf[32];
		_stprintf(bf, Fmt, 0.01f*v);
		SetWindowText(_hVal, bf);
	}
}


void PickColorBase::_Apply()
{
	TCHAR bf[32];
	HWND hEdit = GetFocus();
	if (_hHue == hEdit)
	{
		float v;
		if (_GetEditFloat(hEdit, v))
		{
			_HueSlider.SetValue(static_cast<int>(v*100.0f));
			if (1.0f==v)
				v = 0.0f;
			_hsv.H(360.0f*v);
			_rgb.Set(_hsv);
			_UpdateRGBSliders();
			_UpdateRGBEdits();
			_pCirc->SetColor(_hsv);
			_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
			_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);

			_Modified();
		}
		else
		{
			MessageBeep(0xFFFFFFFF);
			int v = _HueSlider.GetValue();
			_stprintf(bf, Fmt, 0.01f*v);
			SetWindowText(hEdit, bf);
		}
	}
	else if (_hSat == hEdit)
	{
		float v;
		if (_GetEditFloat(hEdit, v))
		{
			_SatSlider.SetValue(static_cast<int>(v*100.0f));
			_hsv.S(v);
			_rgb.Set(_hsv);
			_UpdateRGBSliders();
			_UpdateRGBEdits();
			_pCirc->SetColor(_hsv);

			_Modified();
		}
		else
		{
			MessageBeep(0xFFFFFFFF);
			int v = _HueSlider.GetValue();
			_stprintf(bf, Fmt, 0.01f*v);
			SetWindowText(hEdit, bf);
		}
	}
	else if (_hVal == hEdit)
	{
		float v;
		if (_GetEditFloat(hEdit, v))
		{
			_ValSlider.SetValue(static_cast<int>(v*100.0f));
			_hsv.V(v);
			_rgb.Set(_hsv);
			_UpdateRGBSliders();
			_UpdateRGBEdits();
			_pCirc->SetColor(_hsv);

			_Modified();
		}
		else
		{
			MessageBeep(0xFFFFFFFF);
			int v = _ValSlider.GetValue();
			_stprintf(bf, Fmt, 0.01f*v);
			SetWindowText(hEdit, bf);
		}
	}
	else if (_hRed == hEdit)
	{
		float v;
		if (_GetEditFloat(hEdit, v))
		{
			_RedSlider.SetValue(static_cast<int>(v*100.0f));
			_rgb.R(v);
			_hsv.Set(_rgb);
			_UpdateHSVSliders();
			_UpdateHSVEdits();
			_pCirc->SetColor(_hsv);
			_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
			_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);

			_Modified();
		}
		else
		{
			MessageBeep(0xFFFFFFFF);
			int v = _RedSlider.GetValue();
			_stprintf(bf, Fmt, 0.01f*v);
			SetWindowText(hEdit, bf);
		}
	}
	else if (_hGreen == hEdit)
	{
		float v;
		if (_GetEditFloat(hEdit, v))
		{
			_GreenSlider.SetValue(static_cast<int>(v*100.0f));
			_rgb.G(v);
			_hsv.Set(_rgb);
			_UpdateHSVSliders();
			_UpdateHSVEdits();
			_pCirc->SetColor(_hsv);
			_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
			_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);

			_Modified();
		}
		else
		{
			MessageBeep(0xFFFFFFFF);
			int v = _GreenSlider.GetValue();
			_stprintf(bf, Fmt, 0.01f*v);
			SetWindowText(hEdit, bf);
		}
	}
	else if (_hBlue == hEdit)
	{
		float v;
		if (_GetEditFloat(hEdit, v))
		{
			_BlueSlider.SetValue(static_cast<int>(v*100.0f));
			_rgb.B(v);
			_hsv.Set(_rgb);
			_UpdateHSVSliders();
			_UpdateHSVEdits();
			_pCirc->SetColor(_hsv);
			_pValSlider->SetMaxColor(_hsv.GetHueInRGB(), true);
			_pSatSlider->SetMaxColor(_hsv.GetHueInRGB(), true);

			_Modified();
		}
		else
		{
			MessageBeep(0xFFFFFFFF);
			int v = _BlueSlider.GetValue();
			_stprintf(bf, Fmt, 0.01f*v);
			SetWindowText(hEdit, bf);
		}
	}
}


bool PickColorBase::_GetEditFloat(HWND hEdit, float& v) const
{
	std::string bf;
	Window w(hEdit);
	w.GetText(bf);
	for (std::string::const_iterator it = bf.begin(); it != bf.end(); ++it)
	{
		if (!std::isdigit(*it, std::locale::classic()) && '.' != *it)
			return false;
	}
	v = static_cast<float>(atof(bf.c_str()));
	if (v<0.0f)
		return false;
	else if (v>1.0f)
		return false;
	return true;
}


INIT_COUNTER(PickColorModeless);

PickColorModeless::PickColorModeless(HWND hDlg, LPARAM lParam) : 
FormCtrl(hDlg),
PickColorBase(
			  (reinterpret_cast<const InitData*>(lParam))->_clr, 
			  (reinterpret_cast<const InitData*>(lParam))->_pCB
			 )
{
	_InitialClr = GetColor();
	_Init(Hwnd(), HInstance());
	const InitData* pInit = reinterpret_cast<const InitData*>(lParam);
	if (0 != pInit->_label[0])
		SetWindowText(Hwnd(), pInit->_label);

	RECT r = pInit->_r;
	if (r.left != r.right)
		MoveWindow(r.left, r.top, r.right-r.left, r.bottom-r.top, false);

	ShowWindow(Hwnd(), SW_SHOW);

	INC_COUNTER;
}


PickColorModeless::~PickColorModeless()
{ DEC_COUNTER; }

HWND PickColorModeless::Create(HWND hOwner, const InitData* pInit)
{
	return CreateDialogParam
		(
		App::GetInstance(),
		"PICKCOLOR",
		hOwner, 
		reinterpret_cast<DLGPROC>(PickColorModeless::DlgProc),
		reinterpret_cast<LPARAM>(pInit)
		);
}


BOOL CALLBACK PickColorModeless::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PickColorModeless* pCtrl = GetWinLong<PickColorModeless*>(hDlg);
	switch (msg)
	{
	case WM_INITDIALOG :
		try
		{
			pCtrl = new PickColorModeless(hDlg, lParam);
		}
		catch (Exception e)
		{
			pCtrl->ErrorBox(e);
			DestroyWindow(hDlg);
		}
		return true;
	case WM_DESTROY :
		pCtrl->Dying();
		PickColorModeless::OnDestroy(hDlg);
		return true;
	case WM_COMMAND :
		return pCtrl->Command(static_cast<int>(LOWORD(wParam)), Window(reinterpret_cast<HWND>(lParam)), static_cast<UINT>(HIWORD(wParam)));
	case WM_ACTIVATE :
		return pCtrl->Activate(static_cast<ActiveState>(LOWORD(wParam)), reinterpret_cast<HWND>(lParam));
	case WM_HSCROLL :
		return pCtrl->HScroll(static_cast<Ctrl::HScrollCode>(LOWORD(wParam)), static_cast<int>(HIWORD(wParam)), reinterpret_cast<HWND>(lParam));
	}
	return false;
}


void PickColorModeless::Dying()
{
	RECT r;
	GetWindowRect(r);
	_CallClosedCallback(r);
}


bool PickColorModeless::Command(int id, Window, UINT ncode)
{
	switch (id)
	{
	case IDC_APPLY :
		_Apply();
		break;
	case IDC_RED :
		if (EN_KILLFOCUS == ncode)
			_RChanged();
		break;
	case IDC_GREEN :
		if (EN_KILLFOCUS == ncode)
			_GChanged();
		break;
	case IDC_BLUE :
		if (EN_KILLFOCUS == ncode)
			_BChanged();
		break;
	case IDC_HUE :
		if (EN_KILLFOCUS == ncode)
			_HChanged();
		break;
	case IDC_SATURATION :
		if (EN_KILLFOCUS == ncode)
			_SChanged();
		break;
	case IDC_VALUE :
		if (EN_KILLFOCUS == ncode)
			_VChanged();
		break;
	case IDOK :
	case IDCANCEL :
		_Modified();
		DestroyWindow(Hwnd());
		break;
	case IDC_UNDO:
		_Undo();
		break;
	}
	return true;
}


void PickColorModeless::_Undo()
{
	SetColor(_InitialClr);
	_Modified();
}


bool PickColorModeless::Activate(ActiveState state, HWND)
{
	if (asInactive == state)
		App::SetModeless(0);
	else
		App::SetModeless(Hwnd());
	return true;
}


void PickColorModeless::SetColor(COLORREF clr)
{
	_InitialClr = clr;
	PickColorBase::SetColor(clr);
}



bool PickColorModeless::HScroll(HScrollCode code, int pos, HWND hCtl)
{
	PCBHScroll(Hwnd(), hCtl, code, pos);
	return true;
}


void PickColorModeless::Reassign(const TCHAR* title, COLORREF clr)
{
	SetWindowText(Hwnd(), title);
	SetColor(clr);
}
