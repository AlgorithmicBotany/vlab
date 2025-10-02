#include <iostream>
#include <fw.h>
#include <glfw.h>

#include "menuids.h"
#include "resource.h"
#include "objfgvobject.h"
#include "objfgvview.h"

#include "curveview.h"


void CurveView::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<CurveView>::Proc);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.Register();
}


CurveView::CurveView(HWND hwnd, const CREATESTRUCT* pCS) : OpenGLWindow(hwnd, pCS)
{
	_curveXYZView = new CurveXYZView();

	_pObj = _curveXYZView;

	_curveXYZView->setView(this);

	{
		CurrentContext cc(this);
		_DoInit();
	}
}


CurveView::~CurveView()
{
}


void CurveView::_DoInit()
{
	_curveXYZView->initializeGL();
	_curveXYZView->initGraph();
}


void CurveView::_DoPaint() const
{
	_curveXYZView->paintGL();
}


void CurveView::_DoSize()
{
	_curveXYZView->resizeGL(Width(), Height());
}


bool CurveView::LButtonDown(KeyState ks, int x, int y)
{
	SHORT vkstate = GetKeyState(VK_MENU);

	SetCapture(Hwnd());

	_curveXYZView->mouseLDown(x, y, ks.IsShift(), 
		ks.IsCtrl(), (0x8000 & vkstate) != 0);
	return true;
}


bool CurveView::MouseMove(KeyState, int x, int y)
{
	_curveXYZView->mouseMove(x, y);
	return true;
}


bool CurveView::LButtonUp(KeyState, int x, int y)
{
	_curveXYZView->mouseLUp(x, y);
	ReleaseCapture();

	return true;
}


void CurveView::ContextMenu(HWND, UINT x, UINT y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurvePreviewCMenu);
	TrackPopupMenu(
		hMenu,
		TPM_LEFTALIGN | TPM_TOPALIGN,
		x, y,
		0,
		Hwnd(),
		0);
}


bool CurveView::Command(int id, Window, UINT)
{
	try
	{
		switch (id)
		{
		case ID_CURVEPREVIEW_APPLY :
			_ApplyNow();
			break;
		case ID_CURVEPREVIEW_RETRIEVE :
			_Retrieve();
			break;
		case ID_CURVEPREVIEW_ADDASNEW :
			_AddAsNew();
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}
