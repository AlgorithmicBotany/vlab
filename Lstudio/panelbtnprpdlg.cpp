#include <string>

#include <fw.h>
#include <glfw.h>


#include "panelprms.h"
#include "panelitem.h"
#include "panelbtnprpdlg.h"

#include "resource.h"

PanelButtonPropDlg::PanelButtonPropDlg(const PanelButton* pBtn) : Dialog(IDD_BUTTONPANELPROP),
_name(pBtn->GetName()),
_action(pBtn->Action()),
_outlinebtn(pBtn->Outline(), 0),
_fillbtn(pBtn->Fill(), 0),
_value(pBtn->State())
{}



bool PanelButtonPropDlg::DoInit()
{
	{
		ComboBox list(GetDlgItem(IDC_DEFSTATE));
		_OnItem = list.AddString("On");
		_OffItem = list.AddString("Off");
		_MonostableItem = list.AddString("Monostable");
		switch (_value)
		{
		case PanelParameters::bsOn :
			list.SetCurSel(_OnItem);
			break;
		case PanelParameters::bsOff :
			list.SetCurSel(_OffItem);
			break;
		case PanelParameters::bsMonostable :
			list.SetCurSel(_MonostableItem);
			break;
		}
	}
	{
		ComboBox msg( GetDlgItem(IDC_MESSAGE));
		msg.AddString(__TEXT("n <line> <field> <scale> %d"));
		msg.AddString(__TEXT("d <name> %d <scale>"));
		msg.AddString(__TEXT("o <line> %d"));
	}

	_outlinebtn.Reset(GetDlgItem(IDC_OUTLINE));
	_fillbtn.Reset(GetDlgItem(IDC_FILL));

	return true;
}


bool PanelButtonPropDlg::HandleMsg(HWND hWnd, UINT msg, WPARAM, LPARAM lParam)
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


void PanelButtonPropDlg::_DrawItem(HWND, OwnerDraw::Draw ds)
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



void PanelButtonPropDlg::UpdateData(bool what)
{
	DX(_name, IDC_NAME, what);
	DX(_action, IDC_MESSAGE, what);
	if (what)
	{
		ComboBox list(GetDlgItem(IDC_DEFSTATE));
		int sel = list.GetCurSel();
		if (_OnItem == sel)
			_value = PanelParameters::bsOn;
		else if (_OffItem == sel)
			_value = PanelParameters::bsOff;
		else 
			_value = PanelParameters::bsMonostable;
	}
	else
	{
		ComboBox list(GetDlgItem(IDC_DEFSTATE));
		switch (_value)
		{
		case PanelParameters::bsOn :
			list.SetCurSel(_OnItem);
			break;
		case PanelParameters::bsOff :
			list.SetCurSel(_OffItem);
			break;
		case PanelParameters::bsMonostable :
			list.SetCurSel(_MonostableItem);
			break;
		}
	}
}


bool PanelButtonPropDlg::Command(int id, Window, UINT)
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

