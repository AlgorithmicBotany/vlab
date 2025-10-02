#include <fw.h>
#include <glfw.h>

#include "resource.h"

#include "gridviewtask.h"
#include "gridview.h"


void GridViewTask::LBDblClick(KeyState, int, int)
{}

void GridViewTranslateTask::MouseMove(KeyState, int x, int y)
{
	if (_translating)
	{
		_pView->Translate(x - _lastpos.x, y - _lastpos.y);
		_lastpos.x = x;
		_lastpos.y = y;
	}
	GridViewTask::MouseMove(KeyState(0), x, y);
}

void GridViewTranslateTask::LButtonUp(KeyState, int, int)
{
	ReleaseCapture();
}

void GridViewTranslateTask::LButtonDown(KeyState, int x, int y)
{
	_lastpos.x = x;
	_lastpos.y = y;
	_translating = true;
	SetCapture(_pView->Hwnd());
}

#define ZOOMONY

void GridViewZoomTask::MouseMove(KeyState ks, int x, int y)
{
	if (_zooming)
	{
#ifdef ZOOMONY
		if (y != _lastPos.y)
		{
			_pView->Zoom(y-_lastPos.y);
			_lastPos.x = x;
			_lastPos.y = y;
		}
#else
		if ((x != _lastPos.x) || (y != _lastPos.y))
		{
			POINT delta = {abs(x - _lastPos.x), abs(y - _lastPos.y)};
			int zoom = 0;
			if (delta.x>delta.y)
				zoom = _lastPos.x - x;
			else
				zoom = y - _lastPos.y;
			_pView->Zoom(zoom);
			_lastPos.x = x;
			_lastPos.y = y;
		}
#endif
	}
	GridViewTask::MouseMove(ks, x, y);
}


void GridViewZoomTask::LButtonUp(KeyState, int, int)
{
	ReleaseCapture();
}

void GridViewZoomTask::LButtonDown(KeyState, int x, int y)
{
	_lastPos.x = x;
	_lastPos.y = y;
	_zooming = true;
	SetCapture(_pView->Hwnd());
}

void GridViewZoomTask::MButtonDown(KeyState, int x, int y)
{
	_lastPos.x = x;
	_lastPos.y = y;
	_zooming = true;
	SetCapture(_pView->Hwnd());
}
