#include <fw.h>
#include <glfw.h>

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvedit.h"
#include "prjnotifysnk.h"
#include "contmodedit.h"

#include "patchclrinfo.h"
#include "patch.h"
#include "surface.h"
#include "surfviewtsk.h"
#include "surfaceview.h"

#include "linethcb.h"
#include "surfthumbcb.h"
#include "surfaceedit.h"
#include "menuids.h"
#include "lstudioptns.h"

#include "resource.h"

void SurfaceView::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<SurfaceView>::Proc);
	wc.style = 0;
	wc.hCursor = 0;
	wc.Register();
}


const char* ReqsAdvEd = "Requires advanced editor";
int RqsLength = -1;

SurfaceView::SurfaceView(HWND hwnd, const CREATESTRUCT* pCS) : 
GLTrackball(hwnd, pCS),
_IdleTask(this),
_DragPointTask(this),
_ZoomTask(this),
_PanTask(this)
{
	_scale = 4.0f;
	_theEdit = 0;
	_activePoint = 0;
	_LockXY = false;
	_drawWhat = 
		PatchSpace::DrawMesh | 
		PatchSpace::DrawDenseWireframe | 
		PatchSpace::DrawKnots |
		PatchSpace::DrawKnotNumbers;
	_pObj = new Surface;
	if (-1==RqsLength)
		RqsLength = strlen(ReqsAdvEd);
	SwitchToIdleTask();
	{
		CurrentContext cc(this);
		_DoInit();
	}
}


SurfaceView::~SurfaceView()
{
	CurrentContext cc(this);
	_DoExit();
}


void SurfaceView::CopyObject(const EditableObject* pObj)
{
	ObjectView::CopyObject(pObj);
	const Surface* pSrf = _GetSurface();
	WorldPointf p;
	BoundingBox bb(p);
	pSrf->GetBoundingBox(bb);
	WorldPointf c = bb.Center();
	_centershift.Set(c.X(), c.Y(), c.Z());
	_scale = bb.XSize();
	if (_scale < bb.YSize())
		_scale = bb.YSize();
	_scale *= 1.15f;
	_rotX = _rotY = 0.0f;
	CurrentContext cc(this);
	_DoOther();
	cc.SwapBuffers();
}


void SurfaceView::_SetViewbox()
{
	assert(Height()>0);
	float coeff = static_cast<float>(Width())/static_cast<float>(Height());
	if (Width() > Height())
	{
		_viewbox.SetCX(_centershift.X(), _scale*coeff);
		_viewbox.SetCY(_centershift.Y(), _scale);
		_viewbox.SetZ(-500.0f, 500.0f);
	}
	else
	{
		_viewbox.SetCX(_centershift.X(), _scale);
		_viewbox.SetCY(_centershift.Y(), _scale/coeff);
		_viewbox.SetZ(-500.0f, 500.0f);
	}
}



void SurfaceView::Zoom(int steps)
{
	assert(0 != steps);
	_scale *= powf(1.001f, static_cast<float>(steps));
	CurrentContext cc(this);
	_DoOther();
	cc.SwapBuffers();
}


void SurfaceView::PanBy(int x, int y)
{
	WorldPointf wp(x*_upp, -y*_upp, 0.0f);
	_centershift -= wp;
	CurrentContext cc(this);
	_DoOther();
	cc.SwapBuffers();
}


void SurfaceView::_DoInit()
{
	COLORREF bg = options.GetGridColors()[Options::eBackground];
	glClearColor
		(
		GetRValue(bg)/255.0f, 
		GetGValue(bg)/255.0f, 
		GetBValue(bg)/255.0f, 
		0.0f
		);
	
	glEnable(GL_DEPTH_TEST);
	HDC hDC = wglGetCurrentDC();
	assert(0 != hDC);
	LogFont arial(15, __TEXT("Arial"));
	Font font(arial);
	ObjectHolder sf(hDC, font);
	assert(-1 != RqsLength);
	GetTextExtentPoint32(hDC, ReqsAdvEd, RqsLength, &_sz);
	_fontList.Init(hDC);
}


void SurfaceView::_DoExit()
{
}


void SurfaceView::_DoPaint() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	{
		PushPopMatrix ppm;
		const Surface* pObj = _GetSurface();
		if (pObj->Editable())
			pObj->Draw(_drawWhat, _activePoint, _fontList.Base());
		else
			_DrawNonEditable();
	}

	glFlush();
}

void SurfaceView::_DrawNonEditable() const
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, Width(), 0, Height());
	glListBase(_fontList.Base());
	glColor3f(0.6f, 0.6f, 0.6f);
	glRasterPos3f(0.5f*(Width()-_sz.cx), 0.5f*(Height()-_sz.cy), 0.0f);
	glCallLists(RqsLength, GL_BYTE, ReqsAdvEd);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
}

bool SurfaceView::LButtonDown(KeyState ks, int x, int y)
{
	GrabFocus();
	if (ks.IsShift())
		SwitchToPanTask();
	else if (ks.IsCtrl())
		SwitchToZoomTask();
	else if (_XYLock())
		SwitchToDragPointTask();
	else
		SwitchToRotateTask();
	_pTask->LButtonDown(ks, x, y);

	return true;
}


bool SurfaceView::MButtonDown(KeyState ks, int x, int y)
{
	ks.SetState(KeyState::fCtrl);
	LButtonDown(ks, x, y);
	return true;
}


bool SurfaceView::LButtonUp(KeyState ks, int x, int y)
{
	GLTrackball::LButtonUp(ks, x, y);
	SwitchToIdleTask();
	return true;
}

bool SurfaceView::MButtonUp(KeyState ks, int x, int y)
{
	GLTrackball::LButtonUp(ks, x, y);
	SwitchToIdleTask();
	return true;
}

const WorldPointf& SurfaceView::GetPoint(int n) const
{
	assert(n>=0);
	assert(n<16 || n==ContactPointId);
	const Surface* pObj = _GetSurface();
	if (ContactPointId == n)
		return pObj->GetCP();
	else
		return pObj->GetPoint(n);
}


Surface* SurfaceView::_GetSurface()
{
	return dynamic_cast<Surface*>(_pObj);
}


const Surface* SurfaceView::_GetSurface() const
{
	return dynamic_cast<const Surface*>(_pObj);
}


void SurfaceView::ContextMenu(HWND, UINT x, UINT y)
{
	HMENU hMenu = App::theApp->GetContextMenu(SurfacePreviewCMenu);
	_SetContextMenuChecks(hMenu);
	TrackPopupMenu(
		hMenu,
		TPM_LEFTALIGN | TPM_TOPALIGN,
		x, y,
		0,
		Hwnd(),
		0);
}


void SurfaceView::_SetContextMenuChecks(HMENU hMenu) const
{
	MenuManipulator mm(hMenu);
	mm.SetCheck(ID_SURFACEPREV_LOCKXY, _XYLock());
	mm.SetCheck(ID_SURFACEPREV_DISPLAY_MESH, _drawWhat & PatchSpace::DrawMesh);

	{
		UINT check = 0;
		if (_drawWhat & PatchSpace::DrawWireframe)
			check = ID_SURFACEPREV_DISPLAY_WIREFRAME;
		else if (_drawWhat & PatchSpace::DrawDenseWireframe)
			check = ID_SURFACEPREV_DISPLAY_DENSEWIREFRAME;
		else if (_drawWhat & PatchSpace::DrawVeryDenseWireframe)
			check = ID_SURFACEPREV_DISPLAY_VERYDENSEWIREFRAME;
		else 
			check = ID_SURFACEPREV_DISPLAY_SHADED;
		mm.CheckRadio(ID_SURFACEPREV_DISPLAY_WIREFRAME, ID_SURFACEPREV_DISPLAY_SHADED, check);
	}

	mm.SetCheck(ID_SURFACEPREV_DISPLAY_CONTROLPOINTS, (0 != (_drawWhat & PatchSpace::DrawKnots)));
	mm.SetCheck(ID_SURFACEPREV_DISPLAY_CONTROLPOINTSNMBRS, (0 != (_drawWhat & PatchSpace::DrawKnotNumbers)));
	mm.SetCheck(ID_SURFACEPREV_KEEPSYMMETRIC, _YZSymmetric());
}

bool SurfaceView::_YZSymmetric() const
{
	return _GetSurface()->YZSymmetric();
}



bool SurfaceView::Command(int id, Window, UINT)
{
	try
	{
		switch (id)
		{
		case ID_SURFACEPREV_APPLY :
			_ApplyNow();
			break;
		case ID_SURFACEPREV_RETRIEVE :
			_Retrieve();
			break;
		case ID_SURFACEPREV_ADDASNEW :
			_AddAsNew();
			break;
		case ID_SURFACEPREV_LOCKXY :
			_LockXYDesign();
			break;
		case ID_SURFACEPREV_DISPLAY_MESH :
		case ID_SURFACEPREV_DISPLAY_WIREFRAME :
		case ID_SURFACEPREV_DISPLAY_DENSEWIREFRAME :
		case ID_SURFACEPREV_DISPLAY_VERYDENSEWIREFRAME :
		case ID_SURFACEPREV_DISPLAY_SHADED :
		case ID_SURFACEPREV_DISPLAY_CONTROLPOINTS :
		case ID_SURFACEPREV_DISPLAY_CONTROLPOINTSNMBRS :
			_ToggleDrawParam(id);
			ForceRepaint();
			break;
		case ID_SURFACEPREV_KEEPSYMMETRIC :
			_KeepYZSymmetric();
			break;
		case ID_SURFACEPREV_RESETROTATION :
			ResetRotation();
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


void SurfaceView::_KeepYZSymmetric()
{
	_GetSurface()->ToggleYZSymmetric();
}


void SurfaceView::_ToggleDrawParam(int id)
{
	switch (id)
	{
	case ID_SURFACEPREV_DISPLAY_MESH : // DrawMesh
		if (_drawWhat & PatchSpace::DrawMesh)
			_drawWhat &= ~(PatchSpace::DrawMesh);
		else
			_drawWhat |= PatchSpace::DrawMesh;
		break;
	case ID_SURFACEPREV_DISPLAY_WIREFRAME : // DrawWireframe
		if (_drawWhat & PatchSpace::DrawWireframe)
			_drawWhat &= ~(PatchSpace::DrawWireframe);
		else
			_drawWhat |= PatchSpace::DrawWireframe;
		_drawWhat &= ~(PatchSpace::DrawDenseWireframe | PatchSpace::DrawVeryDenseWireframe | PatchSpace::DrawShaded);
		break;
	case ID_SURFACEPREV_DISPLAY_DENSEWIREFRAME : // DrawDenseWireframe
		if (_drawWhat & PatchSpace::DrawDenseWireframe)
			_drawWhat &= ~(PatchSpace::DrawDenseWireframe);
		else
			_drawWhat |= PatchSpace::DrawDenseWireframe;
		_drawWhat &= ~(PatchSpace::DrawWireframe | PatchSpace::DrawVeryDenseWireframe | PatchSpace::DrawShaded);
		break;
	case ID_SURFACEPREV_DISPLAY_VERYDENSEWIREFRAME : // DrawVeryDenseWireframe
		if (_drawWhat & PatchSpace::DrawVeryDenseWireframe)
			_drawWhat &= ~(PatchSpace::DrawVeryDenseWireframe);
		else
			_drawWhat |= PatchSpace::DrawVeryDenseWireframe;
		_drawWhat &= ~(PatchSpace::DrawWireframe | PatchSpace::DrawDenseWireframe | PatchSpace::DrawShaded);
		break;
	case ID_SURFACEPREV_DISPLAY_SHADED : // DrawShaded
		if (_drawWhat & PatchSpace::DrawShaded)
			_drawWhat &= ~(PatchSpace::DrawShaded);
		else
			_drawWhat |= PatchSpace::DrawShaded;
		_drawWhat &= ~(PatchSpace::DrawWireframe | PatchSpace::DrawDenseWireframe | PatchSpace::DrawVeryDenseWireframe);
		break;
	case ID_SURFACEPREV_DISPLAY_CONTROLPOINTS : // DrawKnots
		if (_drawWhat & PatchSpace::DrawKnots)
			_drawWhat &= ~(PatchSpace::DrawKnots);
		else
			_drawWhat |= PatchSpace::DrawKnots;
		break;
	case ID_SURFACEPREV_DISPLAY_CONTROLPOINTSNMBRS : // DrawKnotNumbers
		if (_drawWhat & PatchSpace::DrawKnotNumbers)
			_drawWhat &= ~(PatchSpace::DrawKnotNumbers);
		else
			_drawWhat |= PatchSpace::DrawKnotNumbers;
		break;
	}
}


void SurfaceView::_LockXYDesign()
{
	if (_XYLock())
		_LockXY = false;
	else
	{
		ResetRotation();
		_LockXY = true;
	}
}



void SurfaceView::PointSelectedForDragging(int i)
{
	assert(i>=0);
	assert(i<16 || ContactPointId==i);
	assert(0 != _theEdit);
	_activePoint = i;
	_theEdit->PointSelected(i);
	{
		CurrentContext cc(this);
		_DoPaint();
		cc.SwapBuffers();
	}
}


int SurfaceView::GetXYClosestPoint(WorldPointf p) const
{
	const Surface* pObj = _GetSurface();
	int toret = ContactPointId;
	float mindist = XYDistance(p, pObj->GetPoint(ContactPointId));
	for (int i=0; i<16; i++)
	{
		float dst = XYDistance(p, pObj->GetPoint(i));
		if (dst<mindist)
		{
			mindist = dst;
			toret = i;
		}
	}
	if (mindist>_upp*5)
		toret = InvalidControlPointId;
	return toret;
}


void SurfaceView::DragPoint(int i, WorldPointf p)
{
	assert(i>=0);
	assert(i<16 || i==ContactPointId);
	assert(0 != _theEdit);
	_GetSurface()->SetPoint(i, p);
	_theEdit->PointMoved(false);
	{
		CurrentContext cc(this);
		_DoPaint();
		cc.SwapBuffers();
	}
}

void SurfaceView::EndDrag()
{
	_theEdit->EndDrag();
}



void SurfaceView::ColorschemeModified()
{
	CurrentContext cc(this);
	COLORREF bg = options.GetGridColors()[Options::eBackground];
	glClearColor
		(
		GetRValue(bg)/255.0f, 
		GetGValue(bg)/255.0f, 
		GetBValue(bg)/255.0f, 
		0.0f
		);
}
