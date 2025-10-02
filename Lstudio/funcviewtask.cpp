#include <fw.h>
#include <glfw.h>

#include "resource.h"

#include "gridviewtask.h"
#include "gridview.h"

#include "objfgvobject.h"
#include "objfgvview.h"


#include "funcviewtask.h"
#include "functionview.h"


FunctionViewTask::FunctionViewTask(FunctionView* pView, Cursor::dc crsr) : 
GridViewTask(pView, crsr)
{}


FunctionViewTask::FunctionViewTask(FunctionView* pView, const char* crsr) : 
GridViewTask(pView, crsr)
{}


FunctionViewDragPointTask::FunctionViewDragPointTask(FunctionView* pView) :
FunctionViewTask(pView, MAKEINTRESOURCE(IDC_CRSS))
{
	_selected = -1;
}



void FunctionViewDragPointTask::LButtonDown(KeyState, int x, int y)
{
	WorldPointf wp;
	_pView->MapScreenToWorld(x, y, wp);
	FunctionView* pView = dynamic_cast<FunctionView*>(_pView);
	_selected = pView->GetClosestPoint(wp);
	_initPos.x = x; _initPos.y = y;

	if (-1 != _selected)
		SetCapture(_pView->Hwnd());
}

void FunctionViewDragPointTask::LBDblClick(KeyState, int x, int y)
{
	WorldPointf wp;
	_pView->MapScreenToWorld(x, y, wp);
	FunctionView* pView = dynamic_cast<FunctionView*>(_pView);
	_selected = pView->GetClosestPoint(wp);
	if (-1 != _selected)
		pView->InputPoint(_selected);
	_selected = -1;
}



void FunctionViewDragPointTask::MouseMove(KeyState, int x, int y)
{
	if (-1 != _selected)
	{
		FunctionView* pView = dynamic_cast<FunctionView*>(_pView);
		pView->MovePointTo(_selected, x, y);
		pView->ForceRepaint();
		pView->Modified(false);
	}
	GridViewTask::MouseMove( KeyState(0), x, y);
}


void FunctionViewDragPointTask::LButtonUp(KeyState, int x, int y)
{
	ReleaseCapture();
	if (-1 != _selected)
	{
		FunctionView* pView = dynamic_cast<FunctionView*>(_pView);
		if ((_initPos.x != x) || (_initPos.y != y))
			pView->Modified(true);
	}
	Reset();
}

FunctionViewAddPointTask::FunctionViewAddPointTask(FunctionView* pView) : 
FunctionViewTask(pView, MAKEINTRESOURCE(IDC_ADDPOINT))
{
	_added = -1;
}

void FunctionViewAddPointTask::LButtonDown(KeyState, int x, int y)
{
	WorldPointf wp;
	FunctionView* pView = dynamic_cast<FunctionView*>(_pView);
	pView->MapScreenToWorld(x, y, wp);

	if (wp.X()<0.0f)
		wp.X(0.0f);
	else if (wp.X()>1.0f)
		wp.X(1.0f);

	_added = pView->AddPoint(wp);
	pView->ForceRepaint();
	pView->Modified(false);
	SetCapture(pView->Hwnd());
}

void FunctionViewAddPointTask::MouseMove(KeyState, int x, int y)
{
	if (-1 != _added)
	{
		FunctionView* pView = dynamic_cast<FunctionView*>(_pView);
		pView->MovePointTo(_added, x, y);
		pView->ForceRepaint();
		pView->Modified(false);
	}
	GridViewTask::MouseMove(KeyState(0), x, y);
}

void FunctionViewAddPointTask::LButtonUp(KeyState, int, int)
{
	if (-1 != _added)
	{
		ReleaseCapture();
		_added = -1;
		FunctionView* pView = dynamic_cast<FunctionView*>(_pView);
		pView->Modified(true);
	}
}

FunctionViewDeletePointTask::FunctionViewDeletePointTask(FunctionView* pView) : 
FunctionViewTask(pView, MAKEINTRESOURCE(IDC_DELETEPOINT))
{}

void FunctionViewDeletePointTask::LButtonDown(KeyState, int x, int y)
{
	WorldPointf wp;
	_pView->MapScreenToWorld(x, y, wp);
	FunctionView* pView = dynamic_cast<FunctionView*>(_pView);
	int selected = pView->GetClosestPoint(wp);
	if (-1 == selected)
		return;
	WorldPointf sp = pView->GetPoint(selected);
	if (pView->CanDelete(selected))
	{
		pView->DeletePoint(selected);
		pView->ForceRepaint();
		pView->Modified(true);
	}
	else
		MessageBeep(0xFFFFFFFF);
}

void FunctionViewDeletePointTask::MouseMove(KeyState, int x, int y)
{
	GridViewTask::MouseMove(KeyState(0), x, y);
}

void FunctionViewDeletePointTask::LButtonUp(KeyState, int, int)
{}


FunctionViewSetPointTask::FunctionViewSetPointTask(FunctionView* pView) :
FunctionViewTask(pView, MAKEINTRESOURCE(IDC_CRSS))
{}


void FunctionViewSetPointTask::LButtonDown(KeyState, int x, int y)
{
	WorldPointf wp;
	_pView->MapScreenToWorld(x, y, wp);
	FunctionView* pView = dynamic_cast<FunctionView*>(_pView);
	int sel = pView->GetClosestPoint(wp);
	if (-1 != sel)
		pView->InputPoint(sel);
}

void FunctionViewSetPointTask::LButtonUp(KeyState, int, int)
{}
