#ifndef __GRIDPREVIEW_H__
#define __GRIDPREVIEW_H__

class GridPreview : public Ctrl
{
public:
	static void Register(HINSTANCE);
	GridPreview(HWND, const CREATESTRUCT*);
	~GridPreview();
	bool Size(SizeState, int, int);
	bool Paint();
	void SetColors(const COLORREF*);
private:
	static const TCHAR* _ClassName()
	{ return __TEXT("GridPreview"); }
	Font _font;
	POINT _range;
	COLORREF _entry[Options::eGridViewEntryCount];
};

#else
	#error File already included
#endif
