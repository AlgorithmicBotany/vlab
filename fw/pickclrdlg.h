#ifndef __PICKCOLORDLG_H__
#define __PICKCOLORDLG_H__


class HueCircle;


class ColorSlider;

class PickColorBase
{
public:


	typedef void (*ChangedCallback)(void*, COLORREF);
	typedef void (*ClosedCallback)(void*, const RECT&);
	struct Callbacks
	{
		Callbacks(ChangedCallback pCh, ClosedCallback pCl, void* pV)
			: _pChng(pCh), _pClsd(pCl), _pV(pV)
		{}
		Callbacks() : _pChng(0), _pClsd(0), _pV(0) {}
		ChangedCallback _pChng;
		ClosedCallback _pClsd;
		void* _pV;
	};

	PickColorBase(COLORREF, const Callbacks*);
	~PickColorBase();

	void PCBHScroll(HWND, HWND, UINT, int);

	static void HueSatCallback(float, float, void*);
	void DoHueSat(float, float);

	COLORREF GetColor() const
	{ return _rgb.Get(); }

	void SetCallbacks(const Callbacks* pCB)
	{ _cb = *pCB; }

	void SetColor(COLORREF);

protected:

	void _RChanged();
	void _GChanged();
	void _BChanged();
	void _HChanged();
	void _SChanged();
	void _VChanged();

	void _Init(HWND, HINSTANCE);
	void _Modified();
	void _Apply();

	bool _GetEditFloat(HWND, float&) const;

	void _UpdateSliders();
	void _UpdateRGBSliders();
	void _UpdateHSVSliders();

	void _UpdateEdits();
	void _UpdateRGBEdits();
	void _UpdateHSVEdits();

	void _CallClosedCallback(const RECT& r)
	{
		if (0 != _cb._pClsd)
			(_cb._pClsd)(_cb._pV, r);
	}
private:
	ColorRGBf _rgb;
	ColorHSVf _hsv;

	HWND _hHue;
	Trackbar _HueSlider;
	HWND _hSat;
	Trackbar _SatSlider;
	ColorSlider* _pSatSlider;
	HWND _hVal;
	Trackbar _ValSlider;
	ColorSlider* _pValSlider;
	HWND _hRed;
	Trackbar _RedSlider;
	HWND _hGreen;
	Trackbar _GreenSlider;
	HWND _hBlue;
	Trackbar _BlueSlider;

	HueCircle* _pCirc;

	HICON _hIcon;

	Callbacks _cb;
};



class PickColorDlg : public Dialog, public PickColorBase
{
public:
	PickColorDlg(COLORREF clr);
	bool DoInit();
	
	bool HandleMsg(HWND, UINT, WPARAM, LPARAM);

	bool Command(int, Window, UINT);
};


class PickColorModeless : public FormCtrl, public PickColorBase
{
public:
	PickColorModeless(HWND, LPARAM);
	~PickColorModeless();

	struct InitData
	{
		InitData(const Callbacks* pCB, const TCHAR* label, COLORREF clr, RECT r) :
			_pCB(pCB), _label(label), _clr(clr), _r(r)
			{}
		const Callbacks* _pCB;
		const TCHAR* _label;
		COLORREF _clr;
		RECT _r;
	};
	static HWND Create(HWND, const InitData*);
	static BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

	bool Command(int, Window, UINT);
	bool Activate(ActiveState, HWND);
	bool HScroll(HScrollCode, int, HWND);

	void Reassign(const TCHAR*, COLORREF);
	void SetColor(COLORREF);
	void Dying();

private:
	void _Undo();

	COLORREF _InitialClr;

	DECLARE_COUNTER;
};

#endif
