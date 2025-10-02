#ifndef __PANELLABELPROPDIALOG_H__
#define __PANELLABELPROPDIALOG_H__


class PanelLabel;

class PanelLabelPropDlg : public Dialog
{
public:
	PanelLabelPropDlg(const PanelLabel*);

	bool DoInit();
	void UpdateData(bool);

	bool HandleMsg(HWND, UINT, WPARAM, LPARAM);
	bool Command(int, Window, UINT);
	const char* Name() const
	{ return _text; }
	COLORREF Color() const
	{ return _color.Color(); }
private:

	void _DrawItem(HWND, OwnerDraw::Draw);

	LongString _text;

	OwnerDrawButton _color;
};


#else
	#error File already included
#endif
