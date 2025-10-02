#include <string>

#include <fw.h>
#include <glfw.h>

#include "panelprms.h"
#include "panelsldprpdlg.h"
#include "panelitem.h"

#include "resource.h"

PanelSliderPropDlg::PanelSliderPropDlg(const PanelSlider* pSlider) :
Dialog(IDD_SLIDERPROP),
_name(pSlider->GetName()),
_action(pSlider->Action()),
_outlinebtn(pSlider->Outline(), 0),
_fillbtn(pSlider->Fill(), 0)
{
	_min = pSlider->MinVal();
	_max = pSlider->MaxVal();
	_val = pSlider->Val();
}


bool PanelSliderPropDlg::DoInit()
{
	ComboBox Cb(GetDlgItem(IDC_MESSAGE));
	Cb.AddString(__TEXT("d <name> %d <scale>"));
	Cb.AddString(__TEXT("<line> <field> %d <scale>"));

	_outlinebtn.Reset(GetDlgItem(IDC_OUTLINE));
	_fillbtn.Reset(GetDlgItem(IDC_FILL));

	return true;
}

void PanelSliderPropDlg::UpdateData(bool what)
{
	DX(_name, IDC_NAME, what);
	DX(_action, IDC_MESSAGE, what);
	DX(_min, IDC_MIN, what);
	DX(_max, IDC_MAX, what);
	DX(_val, IDC_VALUE, what);
}

bool PanelSliderPropDlg::_Check()
{
	if (0 == _name.Length())
	{
		_CheckFailed(IDERR_PANELNAMEEMPTY, IDC_NAME);
		return false;
	}
	if (_name.Length()>PanelParameters::eMaxNameLength)
	{
		Window w(Hdlg());
		if (w.MessageYesNo(IDS_PANELITEMNAMETOOLONG))
			return false;
	}
	if (_action.Length()>PanelParameters::eMaxActionLength)
	{
		_CheckFailed(IDS_MESSAGETOOLONG, IDC_MESSAGE);
		return false;
	}
	if (_min>=_max)
	{
		_CheckFailed(IDERR_MINLTMAX, IDC_MIN);
		return false;
	}

	if ((_val<_min) || (_val>_max))
	{
		_CheckFailed(IDERR_VALOUTOFRANGE, IDC_VALUE);
		return false;
	}

	return true;
}


bool PanelSliderPropDlg::HandleMsg(HWND hWnd, UINT msg, WPARAM, LPARAM lParam)
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


void PanelSliderPropDlg::_DrawItem(HWND, OwnerDraw::Draw ds)
{
	switch (ds.CtrlId())
	{
	case IDC_OUTLINE :
		_outlinebtn.DrawItem(ds);
		break;
	case IDC_FILL :
		_fillbtn.DrawItem(ds);
		break;
	}
}


bool PanelSliderPropDlg::Command(int id, Window, UINT)
{
	switch (id)
	{
	case IDC_OUTLINE :
		_outlinebtn.SelectColor();
		break;
	case IDC_FILL :
		_fillbtn.SelectColor();
		break;
	default :
		return false;
	}
	return true;
}


