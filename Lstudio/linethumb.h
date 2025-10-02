#ifndef __LINETHUMB_H__
#define __LINETHUMB_H__



class LineThumb : public Ctrl
{
public:
	static void Register(HINSTANCE);
	LineThumb(HWND, const CREATESTRUCT*);
	~LineThumb();
	bool Paint();
	bool LButtonDown(KeyState, int, int);
	bool MouseMove(KeyState, int, int);
	bool LButtonUp(KeyState, int, int);
	bool Timer(UINT);


	void MovePoint(int);
	void MovePointTo(int);
	void MoveBkgnd(int);
	void SetCallback(LineThumbCallback* pCallback)
	{ _pCallback = pCallback; }
	void SetX(float);
	void AdjustScale();
	void SetShift(int);

	enum TimerId
	{
		eScrollUp = 1,
			eScrollDown = 2
	};
private:


	void _DrawPoint(Canvas&) const;

	static const TCHAR* _ClassName()
	{ return __TEXT("LineThumb"); }


	static HFONT _hFont;
	static int _counter;

	// Distance between tics in pixels
	// Each tic means 0.1
	enum { LabelStep = 5 }; 

	// Value of middle labeled tic * 100
	// _MidVal = 150 --> 1.5
	int _MidVal;

	// Current value * 100
	// _Value = 235 --> 2.35
	int _Value;

	int XCoordForValue(int val) const
	{ 
		return Width()/2 - LabelStep*(_MidVal-val)/10; 
	}
	int ValueForXCoord(int x) const
	{ 
		return _MidVal + 10*(x-Width()/2)/LabelStep; 
	}
	int XCoordForValue() const
	{ 
		return Width()/2 + LabelStep*(_Value-_MidVal)/10; 
	}
	MovingTask _MovingTask;
	ThumbTask* _pTask;
	LineThumbCallback* _pCallback;

	int _Shift;
};


#else
	#error File already included
#endif
