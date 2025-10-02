#ifndef __PANELGROUPPROPDLG_H__
#define __PANELGROUPPROPDLG_H__


class PanelGroupPropDlg : public Dialog
{
public:
	PanelGroupPropDlg(const PanelGroup*);

	bool DoInit();
	bool HandleMsg(HWND, UINT, WPARAM, LPARAM);
	bool Command(int, Window, UINT);
	COLORREF Color() const
	{ return _color.Color(); }
private:
	void _DrawItem(HWND, OwnerDraw::Draw);

	OwnerDrawButton _color;
};

#else
	#error File already included
#endif
