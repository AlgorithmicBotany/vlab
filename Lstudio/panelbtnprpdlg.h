#ifndef __PANELBUTTONPROPDLG_H__
#define __PANELBUTTONPROPDLG_H__



class PanelButtonPropDlg : public Dialog
{
public:
	PanelButtonPropDlg(const PanelButton*);

	bool DoInit();
	void UpdateData(bool);

	const char* Name() const
	{ return _name; }
	const char* Action() const
	{ return _action; }
	COLORREF Outline() const
	{ return _outlinebtn.Color(); }
	COLORREF Fill() const
	{ return _fillbtn.Color(); }
	PanelParameters::ButtonState Value() const
	{ return _value; }
	bool HandleMsg(HWND, UINT, WPARAM, LPARAM);
	bool Command(int, Window, UINT);
private:
	void _DrawItem(HWND, OwnerDraw::Draw);

	LongString _name;
	LongString _action;
	PanelParameters::ButtonState _value;


	OwnerDrawButton _outlinebtn;
	OwnerDrawButton _fillbtn;

	int _OnItem, _OffItem, _MonostableItem;
};

#else
	#error File already included
#endif
