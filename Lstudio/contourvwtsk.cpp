#include <vector>

#include <fw.h>
#include <glfw.h>

#include "resource.h"

#include "objfgvobject.h"
#include "objfgvview.h"
#include "contour.h"
#include "gridviewtask.h"
#include "gridview.h"
#include "contourvwtsk.h"
#include "contourview.h"


ContourViewTask::ContourViewTask(ContourView* pView, const char* crsr) : 
GridViewTask(pView, crsr)
{}


ContourViewTask::ContourViewTask(ContourView* pView, Cursor::dc crsr) : 
GridViewTask(pView, crsr)
{}


ContourViewDragPointTask::ContourViewDragPointTask(ContourView* pView) :
ContourViewTask(pView, MAKEINTRESOURCE(IDC_CRSS))
{
	_selected = -1;
}



void ContourViewDragPointTask::LButtonDown(KeyState, int x, int y)
{
	WorldPointf wp;
	_pView->MapScreenToWorld(x, y, wp);
	ContourView* pView = dynamic_cast<ContourView*>(_pView);
	_selected = pView->GetClosestPoint(wp);
	SetCapture(_pView->Hwnd());
}


void ContourViewDragPointTask::LBDblClick(KeyState, int x, int y)
{
	WorldPointf wp;
	_pView->MapScreenToWorld(x, y, wp);
	ContourView* pView = dynamic_cast<ContourView*>(_pView);
	_selected = pView->GetClosestPoint(wp);
	if (-1 != _selected)
	{
		if (pView->CanIncMultiplicity(_selected))
		{
			pView->IncMultiplicity(_selected);
			pView->ForceRepaint();
			pView->Modified(false);
		}
		else
			MessageBeep(0xFFFFFFFF);
	}
}



void ContourViewDragPointTask::MouseMove(KeyState, int x, int y)
{
	if (-1 != _selected)
	{
		ContourView* pView = dynamic_cast<ContourView*>(_pView);
		pView->MovePointTo(_selected, x, y);
		pView->ForceRepaint();
		pView->Modified(false);
	}
	GridViewTask::MouseMove(KeyState(0), x, y);
}


void ContourViewDragPointTask::LButtonUp(KeyState, int, int)
{
	ReleaseCapture();
	if (-1 != _selected)
	{
		ContourView* pView = dynamic_cast<ContourView*>(_pView);
		pView->Modified(true);
	}
	Reset();
}

ContourViewAddPointTask::ContourViewAddPointTask(ContourView* pView) : 
ContourViewTask(pView, MAKEINTRESOURCE(IDC_ADDPOINT))
{
	_added = -1;
}

void ContourViewAddPointTask::LButtonDown(KeyState, int x, int y)
{
	ContourView* pView = dynamic_cast<ContourView*>(_pView);
	WorldPointf wp;
	pView->MapScreenToWorld(x, y, wp);
	_added = pView->AddPoint(wp);
	pView->ForceRepaint();
	pView->Modified(false);
	SetCapture(pView->Hwnd());
}

void ContourViewAddPointTask::MouseMove(KeyState, int x, int y)
{
	if (-1 != _added)
	{
		ContourView* pView = dynamic_cast<ContourView*>(_pView);
		pView->MovePointTo(_added, x, y);
		pView->ForceRepaint();
		pView->Modified(false);
	}
	GridViewTask::MouseMove(KeyState(0), x, y);
}

void ContourViewAddPointTask::LButtonUp(KeyState, int, int)
{
	if (-1 != _added)
	{
		ReleaseCapture();
		_added = -1;
		ContourView* pView = dynamic_cast<ContourView*>(_pView);
		pView->Modified(true);
	}
}

ContourViewDeletePointTask::ContourViewDeletePointTask(ContourView* pView) : 
ContourViewTask(pView, MAKEINTRESOURCE(IDC_DELETEPOINT))
{}

void ContourViewDeletePointTask::LButtonDown(KeyState, int x, int y)
{
	WorldPointf wp;
	_pView->MapScreenToWorld(x, y, wp);
	ContourView* pView = dynamic_cast<ContourView*>(_pView);
	int selected = pView->GetClosestPoint(wp);
	if (-1 == selected)
		return;
	WorldPointf sp = pView->GetPoint(selected);
	if (pView->CanDelete(selected))
	{
		pView->DeletePoint(selected);
		pView->ForceRepaint();
		pView->Modified(false);
	}
	else
		MessageBeep(0xFFFFFFFF);
}

void ContourViewDeletePointTask::MouseMove(KeyState, int x, int y)
{
	GridViewTask::MouseMove(KeyState(0), x, y);
}

void ContourViewDeletePointTask::LButtonUp(KeyState, int, int)
{}


ContourViewSetPointTask::ContourViewSetPointTask(ContourView* pView) : 
ContourViewTask(pView, MAKEINTRESOURCE(IDC_CRSS))
{}

void ContourViewSetPointTask::LButtonDown(KeyState, int x, int y)
{
	WorldPointf wp;
	_pView->MapScreenToWorld(x, y, wp);
	ContourView* pView = dynamic_cast<ContourView*>(_pView);
	int sel = pView->GetClosestPoint(wp);
	if (-1 != sel)
		pView->InputPoint(sel);
}


void ContourViewSetPointTask::LButtonUp(KeyState, int, int)
{}

