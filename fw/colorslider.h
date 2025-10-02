#ifndef __COLORSLIDER_H__
#define __COLORSLIDER_H__


#define CSS_NOCOLORBUTTON    0x0001
#define CSS_HUESLIDER        0x0002

class ColorSlider;


class ColorSliderCallback
{
public:
	virtual void ColorChanged(COLORREF, bool) = 0;
};

class SharedPickColor
{
public:
	SharedPickColor();
	~SharedPickColor();
	ColorSlider* CurrentOwner() const;
	void Close();
	HWND SetCurrentOwner(ColorSlider*);
	void Show(bool);
private:
	RECT _rect;
	ColorSlider* _pSlider;
	HWND _hPickColor;

	static void _ColorChangedClbck(void* pV, COLORREF clr)
	{ 
		SharedPickColor* pCtrl = reinterpret_cast<SharedPickColor*>(pV);
		pCtrl->_DoColorChanged(clr);
	}
	void _DoColorChanged(COLORREF clr);
	static void _PickClosedClbck(void* pV, const RECT& r)
	{
		SharedPickColor* pCtrl = reinterpret_cast<SharedPickColor*>(pV);
		pCtrl->_DoPickClosed(r);
	}
	void _DoPickClosed(const RECT&);
};


class ColorSlider : public Ctrl
{
	friend class SharedPickColor;
public:
	ColorSlider(HWND, const CREATESTRUCT*);
	~ColorSlider();

	static void Register(HINSTANCE);
	static void Unregister(HINSTANCE);
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	static const char* _ClassName()
	{ return "ColorSlider"; }

	bool Size(SizeState, int, int);
	bool Paint();
	bool LButtonDown(KeyState, int, int);
	bool MouseMove(KeyState, int, int);
	bool LButtonUp(KeyState, int, int);
	bool CaptureChanged();

	void SetCallback(ColorSliderCallback* pCallback)
	{ _pCallback = pCallback; }
	void SetSharedPickColor(SharedPickColor* pSharedPickColor)
	{ _pSharedPickColor = pSharedPickColor; }

	void SetColor(COLORREF);
	void SetColor(const float*);
	void SetMaxColor(COLORREF, bool = false);
	void SetMinColor(COLORREF, bool = false);
	COLORREF GetColor() const;
	int GetPos() const;
	void SetRange(bool, int, int);
	void SetPos(bool, int);

	void SetLabel(const std::string& label)
	{
		_Label = label;
	}
private:

	const bool _noButton;
	const bool _hueSlider;

	void _ChangeColor();
	void DrawMarker(Canvas&);
	void ColorRangeRepaint(HDC, int) const;
	void _RecalcGradientStep();

	static void _ColorChangedClbck(void* pV, COLORREF clr)
	{ 
		ColorSlider* pCtrl = reinterpret_cast<ColorSlider*>(pV);
		pCtrl->_DoColorChanged(clr);
	}
	void _DoColorChanged(COLORREF clr);
	static void _PickClosedClbck(void* pV, const RECT& r)
	{
		ColorSlider* pCtrl = reinterpret_cast<ColorSlider*>(pV);
		pCtrl->_DoPickClosed(r);
	}
	void _DoPickClosed(const RECT& r)
	{ 
		_hPickColor = 0; 
		_LastPickColorRect = r;
		Invalidate();
	}

	int _Range() const
	{ return _rangemax-_rangemin+1; }
	int _CalcValue() const
	{
		const float relp = 1.0f*_Position/_SlidingWidth;
		return static_cast<int>(relp*_Range()+_rangemin);
	}

	int _Position;
	int _Value;
	COLORREF _minclr;
	COLORREF _clr;
	int _SlidingWidth;
	bool _capturing;

	int _rangemin, _rangemax;
	float _rangestep;

	float _Rstep;
	float _Gstep;
	float _Bstep;

	ColorSliderCallback* _pCallback;
	SharedPickColor* _pSharedPickColor;
	HWND _hParent;
	HWND _hPickColor;
	RECT _LastPickColorRect;
	std::string _Label;

	static const COLORREF _hsv[];
	static const int _hsvCount;
};


#endif
