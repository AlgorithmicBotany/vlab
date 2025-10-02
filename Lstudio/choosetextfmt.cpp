#include <vector>
#include <sstream>

#include <fw.h>
#include <glfw.h>

#include "choosetextfmt.h"
#include "drpdwnpalette.h"
#include "resource.h"


const int ChooseTextFormat::_truetypeSizes[] =
{
	8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72
};


ChooseTextFormat::ChooseTextFormat(const LogFont& lf, COLORREF bg, COLORREF fg)
: Dialog(IDD_CHOOSETXTFMT), 
_lf(lf), 
_bgcb(_BgChangedCallback, this),
_fgcb(_FgChangedCallback, this),
_bg(bg, &_bgcb), 
_fg(fg, &_fgcb), 
_font(_lf), _bgbrush(_bg.Color()),
_Font(Window(0)), _Size(Window(0)),
_Preview(Window(0))
{
	_pApplyClbck = 0;
	_pClbckParam = 0;
}



bool ChooseTextFormat::DoInit()
{
	_Preview.Reset(GetDlgItem(IDC_SAMPLE));
	_Font.Reset(GetDlgItem(IDC_FONT));
	_Size.Reset(GetDlgItem(IDC_FONTSIZE));
	_fg.Reset(GetDlgItem(IDC_FOREGROUND));
	_bg.Reset(GetDlgItem(IDC_BACKGROUND));
	UpdateCanvas cnv(_Preview.Hwnd());
	_hPreviewDC = cnv;
	LOGFONT lf;
	lf.lfCharSet = ANSI_CHARSET;
	_tcscpy(lf.lfFaceName, __TEXT(""));
	lf.lfPitchAndFamily = 0;
	EnumFontFamiliesEx
		(
		_hPreviewDC, &lf,
		_FontEnumProc,
		reinterpret_cast<LPARAM>(this), 0
		);
	_hPreviewDC = 0;
	int ix = _Font.FindString(_lf.lfFaceName);
	_Font.SetCurSel(ix);
	_FontSelected();
	_SizeSelected();
	return true;
}


int CALLBACK ChooseTextFormat::_FontEnumProc
(
 const LOGFONT* pLF, const TEXTMETRIC* pTM,
 DWORD type, LPARAM lParam
)
{
	ChooseTextFormat* pCtrl = reinterpret_cast<ChooseTextFormat*>(lParam);
	return pCtrl->_DoFontEnum(pLF, pTM, type);
}



int ChooseTextFormat::_DoFontEnum(const LOGFONT* pLF, const TEXTMETRIC*, DWORD type)
{
	if (pLF->lfPitchAndFamily & FIXED_PITCH)
	{
		if (type == TRUETYPE_FONTTYPE)
			_AddTrueTypeFont(pLF);
		else if (type == RASTER_FONTTYPE)
		{
			int ix = _Font.AddString(pLF->lfFaceName);
			_Font.SetItemData(ix, _Sizes.size());
			LOGFONT lf = *pLF;
			EnumFontFamiliesEx
				(
				_hPreviewDC, &lf, _FontSizeEnumProc, 
				reinterpret_cast<LPARAM>(this), 0
				);
			_Sizes.push_back(-1);
		}
	}
	return 1;
}


int CALLBACK ChooseTextFormat::_FontSizeEnumProc
(
 const LOGFONT* pLF, const TEXTMETRIC* pTM,
 DWORD type, LPARAM lParam
)
{
	ChooseTextFormat* pCtrl = reinterpret_cast<ChooseTextFormat*>(lParam);
	return pCtrl->_DoFontSizeEnum(pLF, pTM, type);
}

int ChooseTextFormat::_DoFontSizeEnum(const LOGFONT* pLF, const TEXTMETRIC*, DWORD)
{
	_Sizes.push_back(pLF->lfHeight);
	return 1;
}

void ChooseTextFormat::_AddTrueTypeFont(const LOGFONT* pLF)
{
	int ix = _Font.AddString(pLF->lfFaceName);
	_Font.SetItemData(ix, -1);
}


bool ChooseTextFormat::HandleMsg(HWND hWnd, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DRAWITEM :
		_DrawItem(hWnd, OwnerDraw::Draw(reinterpret_cast<const DRAWITEMSTRUCT*>(lParam)));
		break;
	default :
		return false;
	}
	return true;
}


void ChooseTextFormat::_DrawItem(HWND, OwnerDraw::Draw ds)
{
	switch (ds.CtrlId())
	{
	case IDC_FOREGROUND :
		_fg.DrawItem(ds);
		break;
	case IDC_BACKGROUND :
		_bg.DrawItem(ds);
		break;
	}
}

bool ChooseTextFormat::Command(int id, Window ctl, UINT code)
{
	switch (id)
	{
	case IDC_FONT :
		{
			if (CBN_SELCHANGE == code)
				_FontSelected();
		}
		break;
	case IDC_FONTSIZE :
		{
			if (CBN_SELCHANGE == code)
				_SizeSelected();
		}
		break;
	case IDC_FOREGROUND :
		{
			DropDownPalette cc(ctl.Hwnd(), _fg.Color());
			cc.SetChangedCallback(_FgChangedCallback, this);
			COLORREF orig = _fg.Color();
			switch (cc.Choose())
			{
			case DropDownPalette::rtSelected :
				_fg.SetColor(cc.Result());
				_DoFgChanged(cc.Result());
				break;
			case DropDownPalette::rtAborted :
				_fg.SetColor(orig);
				_DoFgChanged(orig);
				break;
			case DropDownPalette::rtPickColor :
				_fg.SelectColor();
				break;
			}
		}
		break;
	case IDC_BACKGROUND :
		{
			DropDownPalette cc(ctl.Hwnd(), _bg.Color());
			cc.SetChangedCallback(_BgChangedCallback, this);
			COLORREF orig = _bg.Color();
			switch (cc.Choose())
			{
			case DropDownPalette::rtSelected :
				_bg.SetColor(cc.Result());
				_DoBgChanged(cc.Result());
				break;
			case DropDownPalette::rtAborted :
				_bg.SetColor(orig);
				_DoBgChanged(orig);
				break;
			case DropDownPalette::rtPickColor :
				_bg.SelectColor();
				break;
			}
		}
		break;
	case IDC_APPLY :
		if (_pApplyClbck != 0)
			_pApplyClbck(_lf, _fg.Color(), _bg.Color(), _pClbckParam);
		break;
	default:
		return false;
	}
	return true;
}


void ChooseTextFormat::_DoBgChanged(COLORREF clr)
{
	_bg.SetColor(clr);
	_bgbrush.Change(clr);
	_bg.Invalidate();
	_Preview.Invalidate();
}


void ChooseTextFormat::_DoFgChanged(COLORREF clr)
{
	_fg.SetColor(clr);
	_fg.Invalidate();
	_Preview.Invalidate();
}

void ChooseTextFormat::_FontSelected()
{
	int prevsize = 0;
	std::string bf;
	{
		int si = _Size.GetCurSel();
		if (-1!=si)
		{
			_Size.GetLBText(si, bf);
			prevsize = _ttoi(bf.c_str());
		}
		else
			prevsize = _lf.lfHeight;
	}
	int ix = _Font.GetCurSel();
	int sz = _Font.GetItemData(ix);
	_Size.ResetContent();
	int sizesel;
	int dif;
	if (sz == -1)
	{
		sizesel = 0;
		dif = abs(prevsize-_truetypeSizes[0]);
		for (int i=0; i<CountOf(_truetypeSizes); ++i)
		{
			std::stringstream tbf;
			tbf << _truetypeSizes[i];
			int newid = _Size.AddString(tbf.str());
			int ndif = abs(_truetypeSizes[i]-prevsize);
			if (ndif<dif)
			{
				dif = ndif;
				sizesel = newid;
			}
		}
	}
	else
	{
		sizesel = 0;
		dif = abs(prevsize-_Sizes.at(0));
		for (int i=0; _Sizes.at(i) != -1; ++i)
		{
			std::stringstream tbf;
			tbf << _Sizes.at(i);
			int newid = _Size.AddString(tbf.str());
			int ndif = abs(prevsize-_Sizes.at(i));
			if (ndif<dif)
			{
				dif = ndif;
				sizesel = newid;
			}
		}
	}
	_Size.SetCurSel(sizesel);
	_UpdatePreview();
}

void ChooseTextFormat::_SizeSelected()
{
	_UpdatePreview();
}

void ChooseTextFormat::_UpdatePreview()
{
	int ix = _Font.GetCurSel();
	std::string bf;
	_Font.GetLBText(ix, bf);
	strncpy(_lf.lfFaceName, bf.c_str(), LF_FACESIZE);
	_lf.lfFaceName[LF_FACESIZE-1] = 0;
	
	ix = _Size.GetCurSel();

	_Size.GetLBText(ix, bf);
	_lf.lfHeight = _ttoi(bf.c_str());
	_font.Change(_lf);
	_Preview.SetFont(_font);
}

HBRUSH ChooseTextFormat::CtlColor(HWND, HDC hDC, HWND hWnd, int)
{
	if (hWnd == _Preview.Hwnd())
	{
		TmpCanvas tc(hDC);
		tc.TextColor(_fg.Color());
		tc.BkColor(_bg.Color());
		return _bgbrush;
	}
	else
		return 0;
}

void CameleonFont::Change(const LogFont& lf)
{
	HGDIOBJ hNF = CreateFontIndirect(&lf);
	if (0 != hNF)
	{
		DeleteObject(GetObj());
		SetObj(hNF);
	}
}

void CameleonBrush::Change(COLORREF clr)
{
	LOGBRUSH lb;
	lb.lbStyle = BS_SOLID;
	lb.lbColor = clr;
	lb.lbHatch = 0;
	HGDIOBJ hbr = CreateBrushIndirect(&lb);
	if (0 != hbr)
	{
		DeleteObject(GetObj());
		SetObj(hbr);
	}
}
