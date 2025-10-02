#ifndef __HUECIRCLE_H__
#define __HUECIRCLE_H__

#ifdef __FW_H__
#error This is not an exported class
#endif

/*

  Hue is [0,1)
  Saturation is [0,1]

*/


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PIf
#define M_PIf 3.14159265358979323846f
#endif



class HueCircle : public Ctrl
{
public:
	static void Register(HINSTANCE);
	static void Unregister(HINSTANCE);

	HueCircle(HWND, const CREATESTRUCT*);
	~HueCircle();
	bool Size(SizeState, int, int);
	bool Paint();
	bool LButtonDown(KeyState, int, int);
	bool MouseMove(KeyState, int, int);
	bool LButtonUp(KeyState, int, int);
	bool CaptureChanged();
	int GetDlgCode();
	bool KeyDown(UINT);

	void SetColor(ColorHSVf);

	typedef void (*NotifyCallback)(float, float, void*);

	void SetNotifyCallback(NotifyCallback pClbck, void* pV)
	{ _pClbck = pClbck; _pClbckParam = pV; }

	static const TCHAR* _ClassName()
	{ return __TEXT("HueCircle"); }
private:

	void _DragTo(int, int);

	void _DrawIndicator(Canvas& cnv) const
	{
		Arc
			(cnv,
			_center.x-_r, _center.y-_r,
			_center.x+_r+1, _center.y+_r+1,
			_center.x, _center.y-_r,
			_center.x, _center.y-_r
			);
		int ex = _center.x+static_cast<int>(cos(_Hue*2.0*M_PI)*_bmpsize/2);
		int ey = _center.y-static_cast<int>(sin(_Hue*2.0*M_PI)*_bmpsize/2);
		cnv.Line(_center.x, _center.y, ex, ey);
	}

	POINT _center;
	POINT _lt;
	int _bmpsize;

	/* 
	_selection contains the coordinates of the 
	selected color relative to the center of the circle
	*/
	POINT _selection;
	HBITMAP _hBmp;
	bool _dragging;
	float _Hue;
	float _Saturation;
	int _r;

	NotifyCallback _pClbck;
	void* _pClbckParam;

	static HBITMAP _hHue0128;
	static HBITMAP _hHue0256;
	static HBITMAP _hHue0512;
};

#endif
