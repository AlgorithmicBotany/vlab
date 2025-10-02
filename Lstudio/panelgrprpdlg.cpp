#include <string>

#include <fw.h>
#include <glfw.h>


#include "panelprms.h"
#include "panelitem.h"
#include "panelgrprpdlg.h"

#include "resource.h"

PanelGroupPropDlg::PanelGroupPropDlg(const PanelGroup* pGroup) :
Dialog(IDD_GROUPPROP),
_color(pGroup->Color(), 0)
{}

bool PanelGroupPropDlg::HandleMsg(HWND hWnd, UINT msg, WPARAM, LPARAM lParam)
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


bool PanelGroupPropDlg::DoInit()
{
	_color.Reset(GetDlgItem(IDC_COLOR));
	return true;
}


void PanelGroupPropDlg::_DrawItem(HWND, OwnerDraw::Draw ds)
{
	switch (ds.CtrlId())
	{
	case IDC_COLOR :
		_color.DrawItem(ds);
		break;
	}
}




bool PanelGroupPropDlg::Command(int id, Window, UINT)
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
