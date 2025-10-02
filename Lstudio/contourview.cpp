#include <memory>
#include <vector>

#include <fw.h>
#include <glfw.h>

#include "resource.h"

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"
#include "prjnotifysnk.h"
#include "contmodedit.h"
#include "menuids.h"
#include "contour.h"
#include "gridviewtask.h"
#include "gridview.h"
#include "contourvwtsk.h"
#include "contourview.h"
#include "contouredit.h"
#include "inputcntrpnt.h"



void ContourView::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<ContourView>::Proc);
	wc.style = CS_DBLCLKS;
	wc.hCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_CRSS));
	wc.Register();
}





ContourView::ContourView(HWND hwnd, const CREATESTRUCT* pCS) : 
GridView(hwnd, pCS),
_DragTask(this),
_AddPointTask(this),
_DeletePointTask(this),
_SetPointTask(this)
{
	_pTheEdit = 0;
	SetScale(1.1f);
	SetCenter(WorldPointf(0.0f, 0.0f, 0.0f));
	_pObj = new Contour;
	{
		CurrentContext cc(this);
		glEnable(GL_POINT_SMOOTH);
	}
}


ContourView::~ContourView()
{}


void ContourView::_DoPaint() const
{
	glClear(GL_COLOR_BUFFER_BIT);

	_PaintGrid();

	const Contour* pObj = _GetContour();
	pObj->Draw(_drawWhat);

	glFlush();
}



void ContourView::MovePointTo(int pnt, int x, int y)
{
	WorldPointf wp;
	MapScreenToWorld(x, y, wp);
	Contour* pObj = _GetContour();
	pObj->MovePoint(pnt, wp);
}


void ContourView::SetDrawPoints(bool set)
{
	if (set)
		_drawWhat |= Contour::DrawPoints;
	else
		_drawWhat &= ~(Contour::DrawPoints);
}


void ContourView::SetDrawSegments(bool set)
{
	if (set)
		_drawWhat |= Contour::DrawSegments;
	else
		_drawWhat &= ~(Contour::DrawSegments);
}


void ContourView::SetDrawCurve(bool set)
{
	if (set)
		_drawWhat |= Contour::DrawCurve;
	else
		_drawWhat &= ~(Contour::DrawCurve);
}


void ContourView::_ResetZoom()
{
	WorldPointf p;
	BoundingBox bb(p);
	Contour* pContour = _GetContour();
	pContour->GetBoundingBox(bb);
	p = bb.Center();
	float mx = bb.XSize();
	float my = bb.YSize();
	if (mx>my)
		SetScale(0.55f*mx);
	else
		SetScale(0.55f*my);
	SetCenter(p);
	CurrentContext cc(this);
	_DoSize();
	_DoPaint();
	cc.SwapBuffers();
}

void ContourView::ContextMenu(HWND, UINT x, UINT y)
{
	HMENU hMenu = App::theApp->GetContextMenu(ContourPreviewCMenu);

	TrackPopupMenu(
		hMenu,
		TPM_LEFTALIGN | TPM_TOPALIGN,
		x, y,
		0,
		Hwnd(),
		0);
}


bool ContourView::Command(int id, Window, UINT)
{
	try
	{
		switch (id)
		{
		case ID_CONTOURPREV_APPLY :
			_ApplyNow();
			break;
		case ID_CONTOURPREV_RETRIEVE :
			_Retrieve();
			break;
		case ID_CONTOURPREV_ADDASNEW :
			_AddAsNew();
			break;
		case ID_CONTOURPREV_FIT :
			_ResetZoom();
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


int ContourView::GetClosestPoint(WorldPointf p) const
{
	const Contour* pContour = _GetContour();
	int ret = pContour->GetClosestPoint(p);
	WorldPointf s = pContour->GetPoint(ret);
	float dist = XYDistance(p, s);
	float scrdist = dist/_upp;
	if (scrdist>5.0f)
		ret = -1;
	return ret;
}


void ContourView::CopyObject(const EditableObject* pObj)
{
	ObjectView::CopyObject(pObj);

	{
		WorldPointf p;
		BoundingBox bb(p);
		Contour* pContour = _GetContour();
		pContour->GetBoundingBox(bb);
		p = bb.Center();
		float mx = bb.XSize();
		float my = bb.YSize();
		if (mx>my)
			SetScale(0.55f*mx);
		else
			SetScale(0.55f*my);
		SetCenter(p);
		CurrentContext cc(this);
		_DoSize();
	}
}

void ContourView::SetClosed(bool set)
{
	_GetContour()->SetClosed(set);
}


void ContourView::Modified(bool final)
{
	_pTheEdit->Modified(final);
}


void ContourView::InputPoint(int i)
{
	Contour* pContour = _GetContour();
	WorldPointf p = pContour->GetPoint(i);

	SetCntrPntDlg dlg(p);
	if (dlg.DoModal(*this))
	{
		p.Set(dlg.GetX(), dlg.GetY(), 0.0f);
		pContour->MovePoint(i, p);
		ForceRepaint();
		Modified(true);
	}
}
