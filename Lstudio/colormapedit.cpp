#include <string>

#include <fw.h>
#include <glfw.h>

#include "prjnotifysnk.h"
#include "colormapedit.h"
#include "colormap.h"
#include "colormapwnd.h"

#include "resource.h"

INIT_COUNTER(ColormapEdit);

HWND ColormapEdit::Create(HWND hParent, HINSTANCE hInst, PrjNotifySink* pNotifySink)
{
	class ColormapEditCreator : public Creator
	{
	public:
		ColormapEditCreator(PrjNotifySink* pNotifySink) : _pNotifySink(pNotifySink)
		{}
		FormCtrl* Create(HWND hDlg)
		{ return new ColormapEdit(hDlg, _pNotifySink); }
	private:
		PrjNotifySink* _pNotifySink;
	};

	ColormapEditCreator creator(pNotifySink);
	return CreateDialogParam
		(
		hInst, 
		MAKEINTRESOURCE(IDD_COLORMAP),
		hParent,
		reinterpret_cast<DLGPROC>(FormCtrl::DlgProc),
		reinterpret_cast<LPARAM>(&creator)
		);
}


ColormapEdit::ColormapEdit(HWND hDlg, PrjNotifySink* pNotifySink) : 
FormCtrl(hDlg),
_pNotifySink(pNotifySink),
_RedSlider(GetDlgItem(IDC_REDSLIDER)),
_GreenSlider(GetDlgItem(IDC_GREENSLIDER)),
_BlueSlider(GetDlgItem(IDC_BLUESLIDER)),
_Red(GetDlgItem(IDC_EDRED)),
_Green(GetDlgItem(IDC_EDGREEN)),
_Blue(GetDlgItem(IDC_EDBLUE)),
_MaterialsButton(GetDlgItem(IDC_MATERIALS)),
_Colormap(GetDlgItem(IDC_COLORMAP)),
_RSlider(GetDlgItem(IDC_REDSLIDER)),
_GSlider(GetDlgItem(IDC_GREENSLIDER)),
_BSlider(GetDlgItem(IDC_BLUESLIDER)),
_REdit(GetDlgItem(IDC_EDRED)),
_GEdit(GetDlgItem(IDC_EDGREEN)),
_BEdit(GetDlgItem(IDC_EDBLUE))
{
	_Filename[0] = 0;
	_InSync = true;

	_pClrWnd = reinterpret_cast<ColormapWnd*>(GetDlgItem(IDC_COLORMAP).GetPtr());
	_pClrWnd->SetColormapEdit(this);

	// Set sliders' range
	_RedSlider.SetRange(0, Colormap::MaxComponentValue);
	_GreenSlider.SetRange(0, Colormap::MaxComponentValue);
	_BlueSlider.SetRange(0, Colormap::MaxComponentValue);
	SetDlgItemInt(Hwnd(), IDC_EDRED, 0, true);
	SetDlgItemInt(Hwnd(), IDC_EDGREEN, 0, true);
	SetDlgItemInt(Hwnd(), IDC_EDBLUE, 0, true);

	COLORREF clr = _pClrWnd->GetColor(0);
	_RedSlider.SetValue(GetRValue(clr));
	{
		ColorSlider* pRSlider = GetWinLong<ColorSlider*>(_RedSlider.Hwnd());
		pRSlider->SetMaxColor(RGB(Colormap::MaxComponentValue, 0, 0));
	}
	_GreenSlider.SetValue(GetGValue(clr));
	{
		ColorSlider* pGSlider = GetWinLong<ColorSlider*>(_GreenSlider.Hwnd());
		pGSlider->SetMaxColor(RGB(0, Colormap::MaxComponentValue, 0));
	}
	_BlueSlider.SetValue(GetBValue(clr));
	{
		ColorSlider* pBSlider = GetWinLong<ColorSlider*>(_BlueSlider.Hwnd());
		pBSlider->SetMaxColor(RGB(0, 0, Colormap::MaxComponentValue));
	}

	SetDlgItemInt(Hwnd(), IDC_OBJID, 0, true);

	SetDlgItemText(Hwnd(), IDC_NAME, Name());

	// Set up constrains
	{
		RECT dlgrect;
		GetWindowRect(dlgrect);
		_MaterialsButton.SetLeft(dlgrect);
		_MaterialsButton.SetWidth();
		_MaterialsButton.SetBottom(dlgrect);
		_MaterialsButton.SetHeight();
		_MaterialsButton.SetMinTop(dlgrect);

		_Colormap.SetLeft(dlgrect);
		_Colormap.SetTop(dlgrect);
		_Colormap.SetRight(dlgrect);
		_Colormap.SetBottom(dlgrect);
		_Colormap.SetMinWidth();
		_Colormap.SetMinHeight();

		_RSlider.SetLeft(dlgrect);
		_RSlider.SetRight(dlgrect);
		_RSlider.SetTop(dlgrect);
		_RSlider.SetHeight();
		_RSlider.SetMinWidth();

		_GSlider.SetLeft(dlgrect);
		_GSlider.SetRight(dlgrect);
		_GSlider.SetTop(dlgrect);
		_GSlider.SetHeight();
		_GSlider.SetMinWidth();

		_BSlider.SetLeft(dlgrect);
		_BSlider.SetRight(dlgrect);
		_BSlider.SetTop(dlgrect);
		_BSlider.SetHeight();
		_BSlider.SetMinWidth();

		_REdit.SetRight(dlgrect);
		_REdit.SetWidth();
		_REdit.SetMinLeft(dlgrect);
		_REdit.SetTop(dlgrect);
		_REdit.SetHeight();

		_GEdit.SetRight(dlgrect);
		_GEdit.SetWidth();
		_GEdit.SetMinLeft(dlgrect);
		_GEdit.SetTop(dlgrect);
		_GEdit.SetHeight();

		_BEdit.SetRight(dlgrect);
		_BEdit.SetWidth();
		_BEdit.SetMinLeft(dlgrect);
		_BEdit.SetTop(dlgrect);
		_BEdit.SetHeight();
	}
	INC_COUNTER;
}

ColormapEdit::~ColormapEdit()
{
	DEC_COUNTER;
}


bool ColormapEdit::HScroll(HScrollCode code, int, HWND hCtl)
{
	int newpos = 0;
	Window w(hCtl);
	Trackbar tb(w);
	newpos = tb.GetValue();
	bool final = true;
	switch (code)
	{
	case hscThumbTrack :
		final = false;
		break;
	case hscLeft :
	case hscRight :
	case hscLineLeft :
	case hscPageLeft :
	case hscLineRight :
	case hscPageRight :
	case hscThumbPosition :
		final = true;
		break;
	case hscEndScroll :
	default :
		return true;
	}
	if (_RedSlider.Hwnd() == hCtl)
		_SetNewRed(newpos);
	else if (_GreenSlider.Hwnd() == hCtl)
		_SetNewGreen(newpos);
	else if (_BlueSlider.Hwnd() == hCtl)
		_SetNewBlue(newpos);
	else assert(!"Unhandled HSCROLL");
	Modified(final);
	return true;
}

bool ColormapEdit::Command(int id, Window, UINT notify)
{
	switch (id)
	{
	case IDC_APPLY :
		_Apply();
		break;
	case IDC_MATERIALS :
		_pNotifySink->UseMaterials();
		break;
	case IDC_EDRED :
		if (EN_SETFOCUS == notify)
			App::SetModeless(*this);
		else if (EN_KILLFOCUS == notify)
		{
			_RedChanged();
			App::ClearModeless();
		}
		break;
	case IDC_EDGREEN :
		if (EN_SETFOCUS == notify)
			App::SetModeless(*this);
		else if (EN_KILLFOCUS == notify)
		{
			_GreenChanged();
			App::ClearModeless();
		}
		break;
	case IDC_EDBLUE :
		if (EN_SETFOCUS == notify)
			App::SetModeless(*this);
		else if (EN_KILLFOCUS == notify)
		{
			_BlueChanged();
			App::ClearModeless();
		}
		break;
		break;
	}
	return true;
}


void ColormapEdit::_SetNewRed(int newval)
{
	assert(newval>=0);
	assert(newval<256);
	_pClrWnd->SetNewRed(newval);
	SetDlgItemInt(Hwnd(), IDC_EDRED, newval, true);
}


void ColormapEdit::_SetNewGreen(int newval)
{
	assert(newval>=0);
	assert(newval<256);
	_pClrWnd->SetNewGreen(newval);
	SetDlgItemInt(Hwnd(), IDC_EDGREEN, newval, true);
}

void ColormapEdit::_SetNewBlue(int newval)
{
	assert(newval>=0);
	assert(newval<256);
	_pClrWnd->SetNewBlue(newval);
	SetDlgItemInt(Hwnd(), IDC_EDBLUE, newval, true);
}


void ColormapEdit::_Apply()
{
	Window w(GetFocus());
	if (w.Is(_Red))
	{
		_RedChanged();
	}
	else if (w.Is(_Green))
	{
		_GreenChanged();
	}
	else if (w.Is(_Blue))
	{
		_BlueChanged();
	}
}

void ColormapEdit::_RedChanged()
{
	int v = _Red.GetInt();

	if (v<0)
	{
		v = 0;
		_Red.SetInt(v);
	}
	else if (v>Colormap::MaxComponentValue)
	{
		v = Colormap::MaxComponentValue;
		_Red.SetInt(v);
	}
	_RedSlider.SetValue(v);
	_pClrWnd->SetNewRed(v);
	Modified(true);
}

void ColormapEdit::_GreenChanged()
{
	int v = _Green.GetInt();

	if (v<0)
	{
		v = 0;
		_Green.SetInt(v);
	}
	else if (v>Colormap::MaxComponentValue)
	{
		v = Colormap::MaxComponentValue;
		_Green.SetInt(v);
	}
	_GreenSlider.SetValue(v);
	_pClrWnd->SetNewGreen(v);
	Modified(true);
}

void ColormapEdit::_BlueChanged()
{
	int v = _Blue.GetInt();

	if (v<0)
	{
		v = 0;
		_Blue.SetInt(v);
	}
	else if (v>Colormap::MaxComponentValue)
	{
		v = Colormap::MaxComponentValue;
		_Blue.SetInt(v);
	}
	_BlueSlider.SetValue(v);
	_pClrWnd->SetNewBlue(v);
	Modified(true);
}

void ColormapEdit::SetSliders(COLORREF clr)
{
	_RedSlider.SetValue(GetRValue(clr));
	SetDlgItemInt(Hwnd(), IDC_EDRED, GetRValue(clr), true);
	_GreenSlider.SetValue(GetGValue(clr));
	SetDlgItemInt(Hwnd(), IDC_EDGREEN, GetGValue(clr), true);
	_BlueSlider.SetValue(GetBValue(clr));
	SetDlgItemInt(Hwnd(), IDC_EDBLUE, GetBValue(clr), true);
}

void ColormapEdit::SetName(const char* fname)
{
	strcpy(_Filename, fname);
	SetDlgItemText(Hwnd(), IDC_NAME, fname);
}

void ColormapEdit::SetCurrent(int id)
{
	SetDlgItemInt(Hwnd(), IDC_OBJID, id, true);
}

void ColormapEdit::Clear()
{
	_pClrWnd->Clear();
	COLORREF rgb = _pClrWnd->Select(0);
	SetCurrent(0);
	SetSliders(rgb);
}


const TCHAR* ColormapEdit::Name() const
{
	if (0 != _Filename[0])
		return _Filename;
	else
		return __TEXT("color.map");
}

void ColormapEdit::Generate() const
{
	WriteBinFile trg(Name());
	BYTE rgb[3];

	COLORREF clr;
	for (int i=0; i<Colormap::NumOfColors; i++)
	{
		clr = _pClrWnd->GetColor(i);
		rgb[0] = GetRValue(clr);
		rgb[1] = GetGValue(clr);
		rgb[2] = GetBValue(clr);
		trg.Write(rgb, sizeof(BYTE), 3);
	}
}


void ColormapEdit::Import(const TCHAR* fname)
{
	_pClrWnd->Import(fname);
	SetName(fname);
	COLORREF clr = _pClrWnd->GetColor(0);
	SetSliders(clr);
}


void ColormapEdit::Export() const
{
	Generate();
}


void ColormapEdit::Timer(HWND, UINT)
{
	if (!_InSync)
	{
		_InSync = _pNotifySink->ColormapModified(false);
		if (_InSync)
			KillTimer(tTimerId);
	}
	else
		KillTimer(tTimerId);
}

void ColormapEdit::Modified(bool final)
{
	if (_pNotifySink->ContinuousMode())
	{
		bool wasinsync = _InSync;
		_InSync = _pNotifySink->ColormapModified(final);
		if (!_InSync)
		{
			if (wasinsync)
				SetTimer(tTimerId, SyncTimerTimeout);
		}
		else
		{
			if (!wasinsync)
				KillTimer(tTimerId);
		}
	}
}


void ColormapEdit::ApplyNow()
{
	_pNotifySink->ColormapModified(true);
}


bool ColormapEdit::Size(SizeState, int w, int h)
{
	_MaterialsButton.Adjust(w, h);
	_Colormap.Adjust(w, h);
	_RSlider.Adjust(w, h);
	_GSlider.Adjust(w, h);
	_BSlider.Adjust(w, h);
	_REdit.Adjust(w, h);
	_GEdit.Adjust(w, h);
	_BEdit.Adjust(w, h);
	return true;
}
