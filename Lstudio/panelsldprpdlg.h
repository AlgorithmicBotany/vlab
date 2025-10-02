#ifndef __PANELSLIDERPROPDIALOG_H__
#define __PANELSLIDERPROPDIALOG_H__


class PanelSlider;

class PanelSliderPropDlg : public Dialog
{
public:
	PanelSliderPropDlg(const PanelSlider*);

	bool DoInit();
	void UpdateData(bool);

	int MinV() const
	{ return _min; }
	int MaxV() const
	{ return _max; }
	int Val() const
	{ return _val; }
	const char* Name() const
	{ return _name; }
	const char* Action() const
	{ return _action; }
	bool HandleMsg(HWND, UINT, WPARAM, LPARAM);
	bool Command(int, Window, UINT);
	COLORREF Outline() const
	{ return _outlinebtn.Color(); }
	COLORREF Fill() const
	{ return _fillbtn.Color(); }
private:
	bool _Check();

	void _DrawItem(HWND, OwnerDraw::Draw);

	LongString _name;
	LongString _action;
	int _min, _max, _val;

	OwnerDrawButton _outlinebtn;
	OwnerDrawButton _fillbtn;
};

#else
	#error File already included
#endif
