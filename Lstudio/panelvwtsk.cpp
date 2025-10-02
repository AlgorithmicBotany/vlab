#include <fw.h>


#include "objfgvview.h"

#include "panelprms.h"
#include "panelvwtsk.h"
#include "pnlitmsel.h"
#include "panelview.h"
#include "panelitem.h"

#include "resource.h"

void PanelViewTask::LButtonDown(KeyState, int, int)
{
	SetCursor(_cursor);
}


void PanelViewTask::LBDblClick(KeyState, int, int)
{}

void PanelViewTask::MouseMove(KeyState, int, int)
{
	SetCursor(_cursor);
}


PanelViewSelectTask::PanelViewSelectTask(PanelView* pView) : PanelViewTask(pView, Cursor::Arrow)
{
	_marking = false;
}

PanelViewSelectTask::~PanelViewSelectTask()
{}

void PanelViewSelectTask::Reset()
{
	SetCursor(_cursor);
	_marking = false;
}

void PanelViewSelectTask::LButtonDown(KeyState ks, int x, int y)
{	
	_pt1.x = x; _pt1.y = y;
	if (ks.IsShift())
	{
		if (_pView->SelectAdd(x, y))
		{
			_pView->Invalidate();
			_marking = false;
		}
	}
	else if (_pView->SelectionEmpty() || !(_pView->InSelection(_pt1)))
	{
		if (_pView->SelectAt(x, y))
		{
			_pView->Invalidate();
			_marking = false;
		}
		else 
		{
			_marking = true;
			_pt2.x = x; _pt2.y = y;
			_pView->DrawMark(_pt1, _pt2);
		}
	}
}

void PanelViewSelectTask::LBDblClick(KeyState, int, int)
{
	_pView->ItemProperties();
}

void PanelViewSelectTask::MouseMove(KeyState ks, int x, int y)
{
	if (_marking)
	{
		POINT np = { x, y };
		_pView->DrawMark(_pt1, _pt2, np);
		_pt2 = np;
	}
	else
	{
		if (ks.IsLButton())
		{
			if (!_pView->SelectionEmpty())
			{
				_pt2.x = x; _pt2.y = y;
				_pView->StartDragging(_pt1, _pt2);
			}
		}
	}
	PanelViewTask::MouseMove(ks, x, y);
}


void PanelViewSelectTask::LButtonUp(KeyState, int, int)
{
	if (_marking)
	{
		_marking = false;
		_pView->DrawMark(_pt1, _pt2);
		_pView->SelectArea(_pt1, _pt2);
	}
}


PanelViewDragItemTask::PanelViewDragItemTask
(PanelView* pView) : PanelViewTask(pView, MAKEINTRESOURCE(IDC_DRAG))
{
}


PanelViewDragItemTask::~PanelViewDragItemTask()
{}

void PanelViewDragItemTask::StartDragging(POINT p1, POINT p2, RECT dragrect)
{
	_dragRect = dragrect;
	_initPos.Set(p1);
	_lastPos.Set(p2);
	_pView->DrawDragRect(_dragRect);
	int dx = p2.x - p1.x; 
	int dy = p2.y - p1.y;
	_pView->DrawDragRect(_dragRect, dx, dy);
	::SetCursor(_cursor);
}

void PanelViewDragItemTask::MouseMove(KeyState ks, int x, int y)
{
	int dx = x - _lastPos.X();
	int dy = y - _lastPos.Y();
	_pView->DrawDragRect(_dragRect, dx, dy);
	_lastPos.Set(x, y);

	PanelViewTask::MouseMove(ks, x, y);
}


void PanelViewDragItemTask::LButtonUp(KeyState, int, int)
{
	int dx = _lastPos.X() - _initPos.X();
	int dy = _lastPos.Y() - _initPos.Y();
	_pView->MoveBy(dx, dy);
	_pView->Invalidate();
	_pView->SwitchToSelect();
}


PanelViewExecuteTask::PanelViewExecuteTask(PanelView* pView) : PanelViewTask(pView, Cursor::Arrow)
{
	_dragging = false;
}


PanelViewExecuteTask::~PanelViewExecuteTask()
{}


void PanelViewExecuteTask::LButtonDown(KeyState, int x, int y)
{
	RECT r = _pView->ExecuteHit(x, y);
	_last.x = x;
	_last.y = y;
	if (r.left != r.right)
	{
		_pView->GrabFocus();
		MapWindowPoints(_pView->Hwnd(), 0, reinterpret_cast<POINT*>(&r), 2);
		ClipCursor(&r);
		_dragging = true;
	}
}


void PanelViewExecuteTask::MouseMove(KeyState ks, int x, int y)
{
	PanelViewTask::MouseMove(ks, x, y);
	_last.x = x;
	_last.y = y;
	if (_dragging)
		_pView->ExecuteDrag(x, y);
}


void PanelViewExecuteTask::LButtonUp(KeyState, int x, int y)
{
	_pView->ExecuteDrag(x, y, true);
	_dragging = false;
	ClipCursor(0);
}

