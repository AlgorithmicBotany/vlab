#include <fw.h>

#include "linethcb.h"
#include "thumbtask.h"
#include "linethumb.h"
#include "resource.h"

int LineThumb::_counter = 0;
HFONT LineThumb::_hFont = 0;

void LineThumb::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<LineThumb>::Proc);
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.hCursor = 0;
	wc.Register();
}



LineThumb::LineThumb(HWND hwnd, const CREATESTRUCT* pCS) : 
Ctrl(hwnd, pCS),
_MovingTask(this)
{
	_pCallback = 0;
	_pTask = 0;
	_MidVal = 0;
	_Value = 0;

	if (0==_counter)
	{
		LogFont lf(15, __TEXT("Arial"));
		_hFont = CreateFontIndirect(&lf);
	}
	_counter++;
}


LineThumb::~LineThumb()
{
	_counter--;

	if (0==_counter)
		DeleteObject(_hFont);
}




const int LineY = 27;

bool LineThumb::Paint()
{
	const int ShortTickLength = 5;
	const int LongTickLength = 10;
	const int TextY = 0;
	const COLORREF pencolor = RGB(0, 255, 0);
	const COLORREF textcolor = RGB(128, 255, 0);
	PaintCanvas pc(Hwnd());
	{
		Pen greenpen(pencolor);
		ObjectHolder sp(pc, greenpen);
		ObjectHolder sf(pc, _hFont);
		pc.BkMode(Canvas::bkTransparent);
		TextAlignment ta(pc, TextAlignment::Top, TextAlignment::Center);
		pc.TextColor(textcolor);
		pc.Line(0, LineY, Width(), LineY);
		const int midx = Width()/2;
		int curx = midx % LabelStep;
		int lb = ValueForXCoord(curx);
		TCHAR bf[8];
		while (curx<Width())
		{
			if (!(lb % 50))
			{
				pc.Line(curx, LineY-LongTickLength, curx, LineY);
				if (abs(lb)<999 || (0==lb % 100))
				{
					_stprintf(bf, __TEXT("%.1f"), lb/100.0);
					pc.TextOut(curx, TextY, bf);
				}
			}
			else
				pc.Line(curx, LineY-ShortTickLength, curx, LineY);
			curx += LabelStep;
			lb+=10;
		}
	}
	_DrawPoint(pc);
	return true;
}


void LineThumb::_DrawPoint(Canvas& cnv) const
{
	Pen yellowpen(RGB(0, 0, 255));
	ObjectHolder sp(cnv, yellowpen);
	ROp rop(cnv, ROp::NotXorPen);
	const int MainX = XCoordForValue();
	cnv.Line(MainX-1, Height()-2, MainX-1, LineY+3);
	cnv.Line(MainX, Height()-2, MainX, LineY+2);
	cnv.Line(MainX+1, Height()-2, MainX+1, LineY+3);
}


bool LineThumb::LButtonDown(KeyState, int x, int y)
{
	if (0 == _pTask)
	{
		_pTask = &_MovingTask;
		_pTask->ButtonDown(x, y);
	}
	return true;
}


bool LineThumb::MouseMove(KeyState, int x, int y)
{
	if (0 != _pTask)
		_pTask->MouseMove(x, y);
	return true;
}


bool LineThumb::LButtonUp(KeyState, int, int)
{
	if (0 != _pTask)
	{
		_pTask->ButtonUp();
		_pTask = 0;
		_pCallback->Moved(_Value/100.0f, true);
	}
	return true;
}


void LineThumb::MovePointTo(int x)
{
	UpdateCanvas cnv(Hwnd());
	_DrawPoint(cnv);
	_Value = ValueForXCoord(x);
	_DrawPoint(cnv);
	assert(0 != _pCallback);
	_pCallback->Moved(_Value/100.0f, false);
}


void LineThumb::MovePoint(int dx)
{
	UpdateCanvas cnv(Hwnd());
	_DrawPoint(cnv);
	const int x = XCoordForValue() + dx;
	_Value = ValueForXCoord(x);
	_DrawPoint(cnv);
	assert(0 != _pCallback);
	_pCallback->Moved(_Value/100.0f, false);
}


void LineThumb::SetX(float x)
{
	UpdateCanvas cnv(Hwnd());
	_DrawPoint(cnv);
	_Value = static_cast<int>(x * 100.0f);
	_DrawPoint(cnv);
}



void LineThumb::MoveBkgnd(int dx)
{
	RECT r = { 0, 0, Width(), Height() };
	ScrollWindowEx
		(
		Hwnd(),
		dx, 0,
		0,
		&r,
		0,
		0,
		SW_ERASE | SW_INVALIDATE
		);
}


void LineThumb::AdjustScale()
{
	int MainX = XCoordForValue();
	if (MainX<0)
	{
		while (MainX<0)
		{
			_MidVal -= 50;
			MainX += 5*LabelStep;
		}
		Invalidate();
	}
	else if (MainX>Width())
	{
		while (MainX>=Width())
		{
			_MidVal += 50;
			MainX -= 5*LabelStep;
		}
		Invalidate();
	}
}


bool LineThumb::Timer(UINT id)
{
	switch (id)
	{
	case eScrollUp :
		_MidVal += _Shift;
		_Value += _Shift;
		_pCallback->Moved(_Value/100.0f, false);
		Invalidate();
		break;
	case eScrollDown :
		_MidVal -= _Shift;
		_Value -= _Shift;
		_pCallback->Moved(_Value/100.0f, false);
		Invalidate();
		break;
	}
	return true;
}

void LineThumb::SetShift(int x)
{
	if (x<20)
		_Shift = 10;
	else if (x<40)
		_Shift = 20;
	else if (x<60)
		_Shift = 40;
	else
		_Shift = 80;
}
