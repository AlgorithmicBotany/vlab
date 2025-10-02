#include <string>

#include <fw.h>
#include <glfw.h>


#include "panelprms.h"
#include "panellblprpdlg.h"
#include "panelitem.h"

#include "resource.h"

PanelLabelPropDlg::PanelLabelPropDlg(const PanelLabel* pLabel) :
Dialog(IDD_LABELPROP),
_text(pLabel->GetName()),
_color(pLabel->Color(), 0)
{}

void PanelLabelPropDlg::UpdateData(bool what)
{
	DX(_text, IDC_TEXT, what);
}



bool PanelLabelPropDlg::HandleMsg(HWND hWnd, UINT msg, WPARAM, LPARAM lParam)
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


void PanelLabelPropDlg::_DrawItem(HWND, OwnerDraw::Draw ds)
{
	switch (ds.CtrlId())
	{
	case IDC_COLOR :
		_color.DrawItem(ds);
		break;
	}
}


bool PanelLabelPropDlg::DoInit()
{
	_color.Reset(GetDlgItem(IDC_OUTLINE));
	return true;
}


bool PanelLabelPropDlg::Command(int id, Window, UINT)
{
	switch (id)
	{
	case IDC_COLOR :
		_color.SelectColor();
		break;
	default :
		return false;
	}
	return true;
}
