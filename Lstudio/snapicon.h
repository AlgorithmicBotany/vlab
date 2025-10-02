#ifndef __SNAPICON_H__
#define __SNAPICON_H__

#include "snaptsk.h"

class SnapIcon : public Ctrl
{
public:
	SnapIcon(HWND, const GenCtrl::CreateData*);
	~SnapIcon();
	static SnapIcon* Create(HWND, POINT);
	bool LButtonDown(KeyState, int, int);
	bool LButtonUp(KeyState, int, int);
	bool CaptureChanged();
	void ContextMenu(HWND, UINT, UINT);
	bool MouseMove(KeyState, int, int);
	bool Size(SizeState, int, int);
	bool Paint();
	bool Command(int, Window, UINT);
private:
	void _SnapNow();
	void _KeepAspectRatio();
	void _FreeSize();

	void BuildRegion();

	static const COLORREF FrameColor;
	Pen _pen;
	UpdateCanvas _cnv;
	const HWND _hPrnt;
	SnapMove _snapMoveTask;
	SnapResize _snapResizeTask;
	SnapTask* _pTask;
	Region _region;
};


#else
	#error File already included
#endif
