#include <fw.h>

#include "lstudioptns.h"
#include "gridpreview.h"

#include "resource.h"

void GridPreview::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<GridPreview>::Proc);
	wc.Register();
}



GridPreview::GridPreview(HWND hWnd, const CREATESTRUCT* pCS) : 
Ctrl(hWnd, pCS),
_font(33, __TEXT("Arial"))
{
	for (int i=0; i<Options::eGridViewEntryCount; i++)
		_entry[i] = RGB(0, 0, 0);
}


GridPreview::~GridPreview()
{}


bool GridPreview::Size(SizeState, int w, int h)
{
	if (w>h)
	{
		_range.x = 200 * w / h;
		_range.y = 200;
	}
	else
	{
		_range.x = 200;
		_range.y = 200 * h / w;
	}
	return true;
}

void GridPreview::SetColors(const COLORREF* arr)
{
	for (int i=0; i<Options::eGridViewEntryCount; i++)
		_entry[i] = arr[i];
}


bool GridPreview::Paint()
{
	PaintCanvas pc(Hwnd());
	{
		RECT r = { 0, 0, Width(), Height() };
		pc.FilledRectangle(r, _entry[Options::eBackground]);
	}

	SetMapMode(pc, MM_ISOTROPIC);
	SetWindowExtEx(pc, _range.x, _range.y, 0);
	SetViewportExtEx(pc, Width()/2, -Height()/2, 0);
	pc.SetOrigin(Width()/2, Height()/2);
	{
		Pen gridpen(_entry[Options::eGrid]);
		ObjectHolder sp(pc, gridpen);
		int x;
		for (x = 40; x<_range.x; x+=40)
			pc.Line(x, -_range.y, x, _range.y);
		for (x = -40; x>=-_range.x; x-=40)
			pc.Line(x, -_range.y, x, _range.y);
		int y;
		for (y = 40; y<_range.y; y+=40)
			pc.Line(-_range.x, y, _range.x, y);
		for (y = -40; y>=-_range.y; y-=40)
			pc.Line(-_range.x, y, _range.x, y);
	}
	{
		Pen axispen(_entry[Options::eAxis]);
		ObjectHolder sp(pc, axispen);
		pc.Line(-_range.x, 0, _range.x, 0);
		pc.Line(0, -_range.y, 0, _range.y);
	}
	static const POINT vrtx[] = 
	{
		{ -140, -140 },
		{ -120,  130 },
		{  -80,  100 },
		{    0,    0 },
		{   40,  -80 },
		{   80, -140 },
		{  100,    0 },
		{  120,   80 },
		{  160,  160 },
		{  190,  100 }
	};
	const int count = CountOf(vrtx);
	{
		Pen sgmpen(_entry[Options::eSegments]);
		ObjectHolder sp(pc, sgmpen);
		Polyline(pc, vrtx, count);
	}
	{
		Pen crvpen(_entry[Options::eCurve]);
		ObjectHolder sp(pc, crvpen);
		PolyBezier(pc, vrtx, count);
	}
	for (int i=0; i<count; i++)
		pc.Set3x3Pixel(vrtx[i].x, vrtx[i].y, _entry[Options::ePoints]);
	{
		ObjectHolder sf(pc, _font);
		pc.BkMode(Canvas::bkTransparent);
		pc.TextColor(_entry[Options::eLabels]);
		TextAlignment ta(pc, TextAlignment::Bottom, TextAlignment::Left);
		for (int x = -_range.x; x<_range.x; x+=80)
			pc.TextOut(x, -200, __TEXT("0.0"));
		for (int y = -160; y<200; y+=40)
			pc.TextOut(-_range.x, y, __TEXT("0.0"));
	}
	return true;
}

