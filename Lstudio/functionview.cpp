#include <vector>

#include <fw.h>
#include <glfw.h>

#include "resource.h"

#include "gridviewtask.h"
#include "gridview.h"

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"

#include "prjnotifysnk.h"
#include "contmodedit.h"

#include "funcpts.h"
#include "function.h"
#include "functionview.h"
#include "funcedit.h"


#include "menuids.h"
#include "lstudioptns.h"

#include "setfncpntdlg.h"


void FunctionView::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<FunctionView>::Proc);
	wc.style = CS_DBLCLKS;
	wc.hCursor = 0;
	wc.Register();
}


FunctionView::FunctionView(HWND hwnd, const CREATESTRUCT* pCS) :
GridView(hwnd, pCS),
_DragTask(this),
_AddPointTask(this),
_DeletePointTask(this),
_SetPointTask(this)
{
	SetScale(0.55f);
	SetCenter(WorldPointf(0.5f, 0.0f, 0.0f));
	_pObj = new Function;

	{
		CurrentContext cc(this);
		glEnable(GL_POINT_SMOOTH);
	}
}


FunctionView::~FunctionView()
{}

void FunctionView::_DoPaint() const
{
	glClear(GL_COLOR_BUFFER_BIT);

	_PaintGrid();

	const Function* pObj = _GetFunction();

	if (_GridDrawWhat & eDrawAxis)
	{
		glColor3fv(options.GetGridColor(Options::eAxis));
		GLlines gll;
		if (pObj->Flipped())
		{
			gll.Vertex(_GridMin.X(), 1.0f);
			gll.Vertex(_GridMax.X(), 1.0f);
		}
		else
		{
			gll.Vertex(1.0f, _GridMin.Y());
			gll.Vertex(1.0f, _GridMax.Y());
		}
	}

	pObj->Draw(_drawWhat);

	glFlush();
}


void FunctionView::MovePointTo(int pnt, int x, int y)
{
	WorldPointf wp;
	MapScreenToWorld(x, y, wp);
	Function* pObj = _GetFunction();
	pObj->MovePoint(pnt, wp);
}



void FunctionView::SetDrawPoints(bool set)
{
	if (set)
		_drawWhat |= FunctionSpace::DrawPoints;
	else
		_drawWhat &= ~(FunctionSpace::DrawPoints);
}


void FunctionView::SetDrawSegments(bool set)
{
	if (set)
		_drawWhat |= FunctionSpace::DrawSegments;
	else
		_drawWhat &= ~(FunctionSpace::DrawSegments);
}


void FunctionView::SetDrawCurve(bool set)
{
	if (set)
		_drawWhat |= FunctionSpace::DrawCurve;
	else
		_drawWhat &= ~(FunctionSpace::DrawCurve);
}



const char* FunctionView::GetName() const
{
	return _GetFunction()->GetName();
}

void FunctionView::SetName(const char* name)
{
	_GetFunction()->SetName(name);
}


void FunctionView::CopyObject(const EditableObject* pObj)
{
	ObjectView::CopyObject(pObj);
	{
		WorldPointf p;
		BoundingBox bb(p);
		Function* pFunction = _GetFunction();
		pFunction->GetBoundingBox(bb);
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

void FunctionView::ContextMenu(HWND, UINT x, UINT y)
{
	HMENU hMenu = App::theApp->GetContextMenu(FunctionPreviewCMenu);
	TrackPopupMenu(
		hMenu,
		TPM_LEFTALIGN | TPM_TOPALIGN,
		x, y,
		0,
		Hwnd(),
		0);
}


bool FunctionView::Command(int id, Window, UINT)
{
	try
	{
		switch (id)
		{
		case ID_FUNCTIONPREVIEW_APPLY :
			_ApplyNow();
			break;
		case ID_FUNCTIONPREVIEW_RETRIEVE :
			_Retrieve();
			break;
		case ID_FUNCTIONPREVIEW_ADD :
			_AddAsNew();
			break;
		case ID_FUNCTIONPREVIEW_FIT :
			_ResetView();
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}

void FunctionView::_ResetView()
{
	WorldPointf p;
	BoundingBox bb(p);
	Function* pFunction = _GetFunction();
	pFunction->GetBoundingBox(bb);
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


int FunctionView::GetClosestPoint(WorldPointf p) const
{
	const Function* pFunction = _GetFunction();
	int ret = pFunction->GetClosestPoint(p);
	WorldPointf s = pFunction->GetPoint(ret);
	float dist = XYDistance(p, s);
	float scrdist = dist/_upp;
	if (scrdist>5.0f)
		ret = -1;

	return ret;
}


int FunctionView::AddPoint(WorldPointf wp)
{
	return _GetFunction()->AddPoint(wp);
}


WorldPointf FunctionView::GetPoint(int i) const
{
	return _GetFunction()->GetPoint(i);
}


bool FunctionView::CanDelete(int i) const
{
	return _GetFunction()->CanDelete(i);
}


void FunctionView::DeletePoint(int i)
{
	_GetFunction()->DeletePoint(i);
}


void FunctionView::Modified(bool final)
{
	_pTheEdit->Modified(final);
}


void FunctionView::_FlipFunction()
{
	_GetFunction()->Flip();
	_ResetView();
	_pTheEdit->Flipped(Flip());
}

bool FunctionView::Flip() const
{
	return _GetFunction()->Flipped();
}

void FunctionView::Flip(bool)
{
	_GetFunction()->Flip();
	_ResetView();
}


void FunctionView::InputPoint(int i)
{
	Function* pFunc = _GetFunction();
	assert(i>=0);
	assert(i<pFunc->NumOfPoints());
	float minv = 0.0f;
	float maxv = 1.0f;
	WorldPointf wp;
	if (0==i)
		maxv = 0.0f;
	else if (pFunc->NumOfPoints()-1 == i)
		minv = 1.0f;
	else
	{
		wp = pFunc->GetPoint(i-1);
		if (pFunc->Flipped())
			minv = wp.Y();
		else
			minv = wp.X();
		wp = pFunc->GetPoint(i+1);
		if (pFunc->Flipped())
			maxv = wp.Y();
		else
			maxv = wp.X();
	}
	wp = pFunc->GetPoint(i);
	if (pFunc->Flipped())
	{
		float tmp = wp.X();
		wp.X(wp.Y());
		wp.Y(tmp);
	}
	SetFuncPntDlg dlg(wp, minv, maxv);
	if (IDOK==dlg.DoModal(*this))
	{
		if (pFunc->Flipped())
			wp.Set(dlg.Y(), dlg.X(), 0.0);
		else
			wp.Set(dlg.X(), dlg.Y(), 0.0);
		pFunc->MovePoint(i, wp);
		ForceRepaint();
		Modified(true);
	}
}


bool FunctionView::ImplementsSamples() const
{
	return _GetFunction()->ImplementsSamples();
}

int FunctionView::GetSamples() const
{
	assert(ImplementsSamples());
	return _GetFunction()->GetSamples();
}


void FunctionView::SetSamples(int i)
{
	assert(ImplementsSamples());
	assert(i>2);
	_GetFunction()->SetSamples(i);
}


const Function* FunctionView::_GetFunction() const
{ return dynamic_cast<const Function*>(_pObj); }


Function* FunctionView::_GetFunction()
{ return dynamic_cast<Function*>(_pObj); }

