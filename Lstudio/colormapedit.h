#ifndef __COLORMAPEDIT_H__
#define __COLORMAPEDIT_H__

class ColormapWnd;

class ColormapEdit : public FormCtrl
{
public:
	ColormapEdit(HWND, PrjNotifySink*);
	~ColormapEdit();

	bool HScroll(HScrollCode, int, HWND);
	bool Command(int, Window, UINT);
	bool Size(SizeState, int, int);

	static HWND Create(HWND, HINSTANCE, PrjNotifySink*);

	void SetCurrent(int);
	void SetSliders(COLORREF);

	void Import(const TCHAR*);
	void Export() const;
	bool IsNamed() const
	{ return 0 != _Filename[0]; }
	void SetName(const TCHAR* fname);
	//{ _tcscpy(_Filename, fname); }

	void Clear();

	void Generate() const;
	const TCHAR* Name() const;
	void Modified(bool);
	void ApplyNow();
	void Timer(HWND, UINT);
private:

	void _Apply();

	void _SetNewRed(int);
	void _SetNewGreen(int);
	void _SetNewBlue(int);

	void _RedChanged();
	void _GreenChanged();
	void _BlueChanged();

	void _Adjust(HWND, GeometryConstrain*, int, int);

	PrjNotifySink* _pNotifySink;
	ColormapWnd* _pClrWnd;
	Trackbar _RedSlider;
	Trackbar _GreenSlider;
	Trackbar _BlueSlider;	
	EditLine _Red;
	EditLine _Green;
	EditLine _Blue;

	TCHAR _Filename[_MAX_PATH+1];

	GeometryConstrain _MaterialsButton;
	GeometryConstrain _Colormap;
	GeometryConstrain _RSlider;
	GeometryConstrain _GSlider;
	GeometryConstrain _BSlider;
	GeometryConstrain _REdit;
	GeometryConstrain _GEdit;
	GeometryConstrain _BEdit;

	bool _InSync;
	enum 
	{ 
		tTimerId = 101,
		SyncTimerTimeout = 100
	};

	DECLARE_COUNTER;
};


#else
	#error File already included
#endif
