#ifndef __MYCHOOSECOLOR_H__
#define __MYCHOOSECOLOR_H__


class DropDownPalette
{
public:
	DropDownPalette(HWND, COLORREF);

	enum ChooseRetType
	{
		rtSelected,
			rtAborted,
			rtPickColor
	};

	ChooseRetType Choose();

	COLORREF Result() const
	{ return _rgb; }

	void Paint(HWND) const;
	void KillFocus(HWND, HWND);
	void Char(HWND, TCHAR, int);
	void MouseMove(HWND, int, int, UINT);
	void LButtonDown(HWND, BOOL, int, int, UINT);
	void Command(HWND, int, Window, UINT);

	void SetChangedCallback(PickColorBase::ChangedCallback pChCb, void* pV)
	{ _cb._pChng = pChCb; _cb._pV = pV; }
	static void Register(HINSTANCE);
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
private:

	void _DrawCell(Canvas&, int, int, COLORREF, bool) const;
	int _GetCell(int, int) const;
	void _DrawFace(Canvas&, int) const;
	void _DrawFace(Canvas&, int, int, COLORREF) const;
	void _DrawFocus(Canvas&, int) const;
	void _DrawFocus(Canvas&, int, int, COLORREF) const;

	void _ChooseColor();
	static const TCHAR* _ClassName()
	{ return __TEXT("MyChooseColor"); }
	bool _stay;
	bool _selected;
	bool _usecc;

	static HINSTANCE _hInst;
	HWND _hMore;
	HWND _hWnd;
	HWND _hParent;
	COLORREF _rgb;
	RECT _InitRect;
	int _current;

	PickColorBase::Callbacks _cb;

	enum Params
	{
		eNumOfRows = 4,
		eNumOfCols = 4,
		ePalletteSize = eNumOfRows*eNumOfCols,
		eCustomColorId = 19,
	};

	static COLORREF _Palette[ePalletteSize];
};


#else
	#error File already included
#endif
