#ifndef __COLORMAPWND_H__
#define __COLORMAPWND_H__

class ColormapEdit;


class ColormapWnd : public Ctrl
{
public:
	ColormapWnd(HWND, const CREATESTRUCT*);
	~ColormapWnd();

	// Message handlers
	bool Size(SizeState, int, int);
	bool Paint();
	int GetDlgCode();
	void ContextMenu(HWND, UINT, UINT);
	bool LButtonDown(KeyState, int, int);
	bool Command(int, Window, UINT);
	bool MouseMove(KeyState, int, int);
	bool KeyDown(UINT);

	static void Register(HINSTANCE);

	void SetColormapEdit(ColormapEdit* pEdit)
	{
		assert(0 == _pEdit);
		_pEdit = pEdit;
	}
	void SetNewRed(int);
	void SetNewGreen(int);
	void SetNewBlue(int);
	COLORREF GetColor(int ix) const
	{
		assert(ix>=0);
		assert(ix<ePaletteSize);
		return _map.Get(ix);
	}
	void Clear() 
	{
		_map.Reset();
		Invalidate();
	}
	COLORREF Select(int ix)
	{
		assert(ix>=0);
		assert(ix<ePaletteSize);
		_SelectCell(ix);
		return GetColor(ix);
	}


	void Import(const TCHAR*); 
	void Generate(WriteBinFile&) const;
private:

	enum Params
	{
		eRows = 16,
		eCols = 16,
		ePaletteSize = Colormap::NumOfColors
	};

	Colormap _map;
	int _cellW;
	int _cellH;

	int _firstActive;
	int _lastActive;

	ColormapEdit* _pEdit;

	bool _IsSelected(int i) const
	{
		assert(i>=0);
		assert(i<ePaletteSize);
		return (i>=_firstActive && i<=_lastActive);
	}
	void _CellRect(int, POINT&);
	void _SelectCell(int, int last=-1);
	void _Smooth()
	{ 
		_map.Smooth(_firstActive, _lastActive); 
		Invalidate();
	}
	void _Inverse()
	{ 
		_map.Inverse(_firstActive, _lastActive);
		Invalidate();
	}
	void _Reverse()
	{
		_map.Reverse(_firstActive, _lastActive);
		Invalidate();
	}
	void _Repaint(int);

	// If outside used area returns -1
	int _CellFromPoint(int, int);
	void _MarkSelected(Canvas&, bool white = true);
	static const TCHAR* _ClassName()
	{ return __TEXT("ColormapWnd"); }
	DECLARE_COUNTER;
};


#else
	#error File already included
#endif
