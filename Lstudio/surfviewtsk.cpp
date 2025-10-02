#include <fw.h>
#include <glfw.h>

#include "surfviewtsk.h"

#include "objfgvobject.h"
#include "objfgvview.h"
#include "patchclrinfo.h"
#include "patch.h"
#include "surface.h"
#include "surfaceview.h"

#include "resource.h"


SurfaceViewZoomTask::SurfaceViewZoomTask(GLTrackball* pView) : GLTask(pView, MAKEINTRESOURCE(IDC_ZOOM))
{
	_theView = dynamic_cast<SurfaceView*>(pView);
}


void SurfaceViewZoomTask::LButtonDown(KeyState, int, int y)
{
	SetCapture(_pView->Hwnd());
	_LastPos.y = y;
	_zooming = true;
}


void SurfaceViewZoomTask::MouseMove(KeyState, int x, int y)
{
	if (_zooming)
	{
		if (y != _LastPos.y)
		{
			_theView->Zoom(y-_LastPos.y);
			_LastPos.y = y;
		}
	}
	GLTask::MouseMove(KeyState(0), x, y);
}

void SurfaceViewZoomTask::LButtonUp(KeyState, int, int)
{
	if (_zooming)
	{
		ReleaseCapture();
		_zooming = false;
		_theView->SwitchToRotateTask();
	}
}


SurfaceViewDragPointTask::SurfaceViewDragPointTask(GLTrackball* pView) : GLTask(pView, MAKEINTRESOURCE(IDC_CRSS))
{
	_theView = dynamic_cast<SurfaceView*>(pView);
	_selectedPoint = InvalidControlPointId;
}


void SurfaceViewDragPointTask::LButtonDown(KeyState, int x, int y)
{
	WorldPointf wp;
	_pView->MapScreenToWorld(x, y, wp);
	_selectedPoint = _theView->GetXYClosestPoint(wp);
	if (InvalidControlPointId != _selectedPoint)
	{
		WorldPointf selected = _theView->GetPoint(_selectedPoint);
		_dragZ = selected.Z();
		_theView->PointSelectedForDragging(_selectedPoint);
		SetCapture(_pView->Hwnd());
	}
}


void SurfaceViewDragPointTask::LButtonUp(KeyState, int, int)
{
	if (InvalidControlPointId != _selectedPoint)
	{
		ReleaseCapture();
		_selectedPoint = InvalidControlPointId;
		_theView->EndDrag();
	}
}

void SurfaceViewDragPointTask::MouseMove(KeyState, int x, int y)
{
	if (InvalidControlPointId != _selectedPoint)
	{
		WorldPointf wp;
		_pView->MapScreenToWorld(x, y, wp);
		wp.Z(_dragZ);
		_theView->DragPoint(_selectedPoint, wp);
	}
	GLTask::MouseMove(KeyState(0), x, y);
}


SurfaceViewPanTask::SurfaceViewPanTask(SurfaceView* pView) : GLTask(pView, MAKEINTRESOURCE(IDC_PAN))
{
	_theView = pView;
	_panning = false;
}

void SurfaceViewPanTask::LButtonDown(KeyState, int x, int y)
{
	SetCapture(_pView->Hwnd());
	_LastPos.x = x;
	_LastPos.y = y;
	_panning = true;
}

void SurfaceViewPanTask::LButtonUp(KeyState, int, int)
{
	ReleaseCapture();
	_panning = false;
}

void SurfaceViewPanTask::MouseMove(KeyState, int x, int y)
{
	if (_panning)
	{
		_theView->PanBy(x-_LastPos.x, y-_LastPos.y);
		_LastPos.x = x;
		_LastPos.y = y;
	}
	GLTask::MouseMove(KeyState(0), x, y);
}

