#ifndef __VIEWOPTIONSDLG_H__
#define __VIEWOPTIONSDLG_H__


class GridPreview;

class ViewOptionsDlg : public Dialog
{
public:
	ViewOptionsDlg(const LStudioOptions&);
	void UpdateData(bool);
	bool DoInit();
	bool HandleMsg(HWND, UINT, WPARAM, LPARAM);
	bool Command(int, Window, UINT);
	const COLORREF* GetGridColors() const
	{ return _entry; }
	const float* GetGridWidths() const
	{ return _aWidth; }

	typedef void (*ApplyCallback)(const COLORREF*, const float*, void*);

	void SetApplyCallback(ApplyCallback pClbck, void* pV)
	{ _pClbck = pClbck; _pClbckParam = pV; }
private:

	static void _ClrChangedCallback(void* pV, COLORREF clr)
	{
		ViewOptionsDlg* pSelf = reinterpret_cast<ViewOptionsDlg*>(pV);
		pSelf->_DoClrChanged(clr);
	}
	void _DoClrChanged(COLORREF);
	void _ItemChanged(int);

	OwnerDrawButton::ColorChangedCallback _ccb;
	OwnerDrawButton _color;

	GridPreview* _pPreview;

	ComboBox _Item;
	EditLine _Width;
	int _selection;

	float _width;
	float _aWidth[Options::wGridWidthEntryCount];
	COLORREF _entry[Options::eGridViewEntryCount];
	void _DrawItem(HWND, OwnerDraw::Draw);
	void _Color(HWND);

	static const UINT _dropentries[];

	ApplyCallback _pClbck;
	void* _pClbckParam;
};


#else
	#error File already included
#endif
