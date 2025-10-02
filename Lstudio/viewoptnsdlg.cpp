#include <fw.h>

#include "lstudioptns.h"


#include "gridpreview.h"
#include "viewoptnsdlg.h"

#include "resource.h"

const UINT ViewOptionsDlg::_dropentries[] =
{
	IDS_BACKGROUND,
		IDS_GRID,
		IDS_AXIS,
		IDS_CURVE,
		IDS_SEGMENTS,
		IDS_POINTS,
		IDS_LABELS
};


ViewOptionsDlg::ViewOptionsDlg(const LStudioOptions& options) : 
Dialog(IDD_VIEWOPTIONS),
_ccb(_ClrChangedCallback, this),
_color(RGB(255, 0, 0), &_ccb),
_Item(Window(0))
{
	for (int i=0; i<Options::eGridViewEntryCount; i++)
		_entry[i] = options.GetGridColors()[i];

	_aWidth[Options::wCurveWidth] = options.GetCurveWidth();
	_aWidth[Options::wSegmentWidth] = options.GetSegmentsWidth();
	_aWidth[Options::wPointSize] = options.GetPointSize();

	_width=0.0f;
	_pClbck = 0; _pClbckParam = 0;
}


bool ViewOptionsDlg::DoInit()
{
	_Item.Reset(GetDlgItem(IDC_ITEM));
	ResString lbl(128, _dropentries[0]);
	for (int i=0; i<CountOf(_dropentries); i++)
	{
		lbl.Load(_dropentries[i]);
		_Item.AddString(lbl);
	}
	_Item.SetCurSel(0);
	_selection = 0;
	_color.SetColor(_entry[Options::eBackground]);
	_pPreview = reinterpret_cast<GridPreview*>(GetDlgItem(IDC_GRIDPREVIEW).GetPtr());
	_pPreview->SetColors(_entry);
	_Width.Reset(GetDlgItem(IDC_WIDTH));
	_Width.Enable(false);

	_color.Reset(GetDlgItem(IDC_COLOR));

	return true;
}



bool ViewOptionsDlg::HandleMsg(HWND hWnd, UINT msg, WPARAM, LPARAM lParam)
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


void ViewOptionsDlg::_DrawItem(HWND, OwnerDraw::Draw ds)
{
	switch (ds.CtrlId())
	{
	case IDC_COLOR :
		_color.DrawItem(ds);
		break;
	}
}


bool ViewOptionsDlg::Command(int id, Window ctl, UINT notify)
{
	switch (id)
	{
	case IDC_COLOR :
		_color.SelectColor();
		break;
	case IDC_ITEM :
		if (CBN_SELCHANGE == notify)
		{
			ComboBox cb(ctl);
			int sel = cb.GetCurSel();
			_ItemChanged(sel);
		}
		break;
	case IDC_APPLY :
		if (0 != _pClbck)
		{
			UpdateData(true);
			_pClbck(_entry, _aWidth, _pClbckParam);
		}
		break;
	default :
		return false;
	}
	return true;
}


void ViewOptionsDlg::_ItemChanged(int sel)
{
	if (sel != _selection)
	{
		UpdateData(true);
		_color.SetColor(_entry[sel]);
		_color.Invalidate();

		if (sel == Options::eCurve || sel == Options::eSegments || sel == Options::ePoints)
			_Width.Enable(true);
		else
			_Width.Enable(false);
		_selection = sel;

		UpdateData(false);
	}
}

void ViewOptionsDlg::_Color(HWND hCtl)
{
	Choosecolor cc(hCtl, _color.Color());
	if (cc.Choose())
	{
		_color.SetColor(cc.rgbResult);
		_color.Invalidate();
		int sel = _Item.GetCurSel();
		if (CB_ERR != sel)
		{
			assert(sel>=Options::eBackground);
			assert(sel<=Options::eLabels);
			_entry[sel] = cc.rgbResult;
			_pPreview->SetColors(_entry);
			_pPreview->Invalidate();
		}
	}
}


void ViewOptionsDlg::_DoClrChanged(COLORREF clr)
{
	int sel = _Item.GetCurSel();
	if (CB_ERR != sel)
	{
		assert(sel>=Options::eBackground);
		assert(sel<=Options::eLabels);
		_entry[sel] = clr;
		_pPreview->SetColors(_entry);
		_pPreview->Invalidate();
	}
}

void ViewOptionsDlg::UpdateData(bool which)
{
	if (_selection == Options::eCurve)
		_width = _aWidth[Options::wCurveWidth];
	else if (_selection == Options::eSegments)
		_width = _aWidth[Options::wSegmentWidth];
	else if (_selection == Options::ePoints)
		_width = _aWidth[Options::wPointSize];

	float v = _width;
	DX(v, IDC_WIDTH, which);
	if (which)
	{
		_width = static_cast<float>(v);
		if (_selection == Options::eCurve)
			_aWidth[Options::wCurveWidth] = _width;
		else if (_selection == Options::eSegments)
			_aWidth[Options::wSegmentWidth] = _width;
		else if (_selection == Options::ePoints)
			_aWidth[Options::wPointSize] = _width;
	}
}

