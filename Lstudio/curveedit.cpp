#include <memory>
#include <iostream>

#include <fw.h>
#include <glfw.h>

#include "menuids.h"

#include "objfgvobject.h"
#include "objfgvedit.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "glgallery.h"
#include "resource.h"

#include "prjnotifysnk.h"
#include "curveview.h"
#include "curveedit.h"
#include "curvegallery.h"

CurveEdit::CurveEdit(HWND hwnd, HINSTANCE hInst, PrjNotifySink* pNotifySink) : FormCtrl(hwnd),
_iInitBSpline(hInst, MAKEINTRESOURCE(IDI_CRV_INIT_BSPLINE)),
_iInitBzSurface(hInst, MAKEINTRESOURCE(IDI_CRV_INIT_BZSURFACE)),
_iOpen(hInst, MAKEINTRESOURCE(IDI_CRV_OPEN)),
_iSave(hInst, MAKEINTRESOURCE(IDI_CRV_SAVE)),
_iInitView(hInst, MAKEINTRESOURCE(IDI_CRV_INIT_VIEW)),
_iInitFit(hInst, MAKEINTRESOURCE(IDI_CRV_INIT_FIT)),
_iUndo(hInst, MAKEINTRESOURCE(IDI_CRV_UNDO)),
_iRedo(hInst, MAKEINTRESOURCE(IDI_CRV_REDO)),
_iPointer(hInst, MAKEINTRESOURCE(IDI_CRV_POINTER)),
_iRotate(hInst, MAKEINTRESOURCE(IDI_CRV_ROTATE)),
_iScale(hInst, MAKEINTRESOURCE(IDI_CRV_SCALE)),
_iTranslate(hInst, MAKEINTRESOURCE(IDI_CRV_TRANSLATE)),
_iAxes(hInst, MAKEINTRESOURCE(IDI_CRV_AXES)),
_iGrid(hInst, MAKEINTRESOURCE(IDI_CRV_GRID)),
_iInsert(hInst, MAKEINTRESOURCE(IDI_CRV_INSERT)),
_iDelete(hInst, MAKEINTRESOURCE(IDI_CRV_DELETE)),
_iMult(hInst, MAKEINTRESOURCE(IDI_CRV_MULT)),
_iType(hInst, MAKEINTRESOURCE(IDI_CRV_TYPE)),
_iSelect(hInst, MAKEINTRESOURCE(IDI_CRV_SELECT)),
_iAlign(hInst, MAKEINTRESOURCE(IDI_CRV_ALIGN)),
_iDPoints(hInst, MAKEINTRESOURCE(IDI_CRV_DPOINTS)),
_iDConvex(hInst, MAKEINTRESOURCE(IDI_CRV_DCONVEX)),
_iDCurve(hInst, MAKEINTRESOURCE(IDI_CRV_DCURVE)),
_iDivide(hInst, MAKEINTRESOURCE(IDI_CRV_DIVIDE)),
_iShow(hInst, MAKEINTRESOURCE(IDI_CRV_SHOW)),
_iHide(hInst, MAKEINTRESOURCE(IDI_CRV_HIDE)),
_iBright(hInst, MAKEINTRESOURCE(IDI_CRV_BRIGHT)),
_iDark(hInst, MAKEINTRESOURCE(IDI_CRV_DARK)),
_iTurtle(hInst, MAKEINTRESOURCE(IDI_CRV_TURTLE)),
_iTransform(hInst, MAKEINTRESOURCE(IDI_CRV_TRANSFORM)),
_iSPaper(hInst, MAKEINTRESOURCE(IDI_CRV_SPAPER)),
_iFlip(hInst, MAKEINTRESOURCE(IDI_CRV_FLIP)),
_iCAGD(hInst, MAKEINTRESOURCE(IDI_CRV_CAGD))
#ifdef CURVEEDITRESIZABLE
,
_ViewWnd(GetDlgItem(IDC_VIEW)),
_Gallery(GetDlgItem(IDC_GALLERY)),
_NameLbl(GetDlgItem(IDC_NAMELBL)),
_Name(GetDlgItem(IDC_NAME)),
_StatWnd(GetDlgItem(IDC_CRV_STATUS)),
_gSimple(GetDlgItem(IDC_SIMPLE)),
_gInitBSpline(GetDlgItem(IDC_CRV_INIT_BSPLINE)),
_gInitBzSurface(GetDlgItem(IDC_CRV_INIT_BZSURFACE)),
_gOpen(GetDlgItem(IDC_CRV_OPEN)),
_gSave(GetDlgItem(IDC_CRV_SAVE)),
_gInitView(GetDlgItem(IDC_CRV_INIT_VIEW)),
_gInitFit(GetDlgItem(IDC_CRV_INIT_FIT)),
_gUndo(GetDlgItem(IDC_CRV_UNDO)),
_gRedo(GetDlgItem(IDC_CRV_REDO)),
_gPointer(GetDlgItem(IDC_CRV_POINTER)),
_gRotate(GetDlgItem(IDC_CRV_ROTATE)),
_gScale(GetDlgItem(IDC_CRV_SCALE)),
_gTranslate(GetDlgItem(IDC_CRV_TRANSLATE)),
_gAxes(GetDlgItem(IDC_CRV_AXES)),
_gGrid(GetDlgItem(IDC_CRV_GRID)),
_gInsert(GetDlgItem(IDC_CRV_INSERT)),
_gDelete(GetDlgItem(IDC_CRV_DELETE)),
_gMult(GetDlgItem(IDC_CRV_MULT)),
_gType(GetDlgItem(IDC_CRV_TYPE)),
_gSelect(GetDlgItem(IDC_CRV_SELECT)),
_gAlign(GetDlgItem(IDC_CRV_ALIGN)),
_gDPoints(GetDlgItem(IDC_CRV_DPOINTS)),
_gDConvex(GetDlgItem(IDC_CRV_DCONVEX)),
_gDCurve(GetDlgItem(IDC_CRV_DCURVE)),
_gDivide(GetDlgItem(IDC_CRV_DIVIDE)),
_gShow(GetDlgItem(IDC_CRV_SHOW)),
_gHide(GetDlgItem(IDC_CRV_HIDE)),
_gBright(GetDlgItem(IDC_CRV_BRIGHT)),
_gDark(GetDlgItem(IDC_CRV_DARK)),
_gTurtle(GetDlgItem(IDC_CRV_TURTLE)),
_gTransform(GetDlgItem(IDC_CRV_TRANSFORM)),
_gSPaper(GetDlgItem(IDC_CRV_SPAPER)),
_gFlip(GetDlgItem(IDC_CRV_FLIP)),
_gCAGD(GetDlgItem(IDC_CRV_CAGD))
#endif
{
	_pNotifySink = pNotifySink;

	//HWND     Control;
	TOOLINFO ti;

	_hwndTT = CreateWindow(TOOLTIPS_CLASS, (LPSTR) 0, TTS_ALWAYSTIP, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
		Hwnd(), (HMENU) 0, hInst, 0); 
	
	_curveView = reinterpret_cast<CurveView*>(GetDlgItem(IDC_VIEW).GetPtr());
	_curveXYZView = _curveView->getCurveXYZView();
	_curveXYZView->setEdit(this);

	_pView = _curveView;
	_pView->SetEdit(this);
	_pGallery = reinterpret_cast<CurveGallery*>(GetDlgItem(IDC_GALLERY).GetPtr());
	_pGallery->SetEdit(this);

	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags =(TTF_IDISHWND | TTF_SUBCLASS);
	ti.hwnd = Hwnd();
	ti.hinst = hInst;

	Button button(GetDlgItem(IDC_CRV_INIT_BSPLINE));
	button.SetIcon(_iInitBSpline);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Init B-Spline";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_INIT_BZSURFACE));
	button.SetIcon(_iInitBzSurface);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Init Bz-Surface";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_OPEN));
	button.SetIcon(_iOpen);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Open";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_SAVE));
	button.SetIcon(_iSave);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Save As";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_INIT_VIEW));
	button.SetIcon(_iInitView);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Init View";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_INIT_FIT));
	button.SetIcon(_iInitFit);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Fit View";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_UNDO));
	button.SetIcon(_iUndo);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Undo";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_REDO));
	button.SetIcon(_iRedo);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Redo";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_POINTER));
	button.SetIcon(_iPointer);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Select";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_ROTATE));	
	button.SetIcon(_iRotate);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Rotate";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_SCALE));
	button.SetIcon(_iScale);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Zoom";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_TRANSLATE));
	button.SetIcon(_iTranslate);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Translate";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_AXES));
	button.SetIcon(_iAxes);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Axes";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_GRID));
	button.SetIcon(_iGrid);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Grid...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_INSERT));
	button.SetIcon(_iInsert);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Insert...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_DELETE));
	button.SetIcon(_iDelete);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Delete...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_MULT));
	button.SetIcon(_iMult);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Multiplicity...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_TYPE));
	button.SetIcon(_iType);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Type...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_SELECT));
	button.SetIcon(_iSelect);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Select...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_ALIGN));
	button.SetIcon(_iAlign);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Align";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_DPOINTS));
	button.SetIcon(_iDPoints);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Display Points";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_DCONVEX));
	button.SetIcon(_iDConvex);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Display Convex";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_DCURVE));
	button.SetIcon(_iDCurve);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Display Curve";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_DIVIDE));
	button.SetIcon(_iDivide);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Divide...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_SHOW));
	button.SetIcon(_iShow);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Show...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_HIDE));
	button.SetIcon(_iHide);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Hide...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_BRIGHT));
	button.SetIcon(_iBright);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Bright...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_DARK));
	button.SetIcon(_iDark);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Dark...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_TURTLE));
	button.SetIcon(_iTurtle);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Turtle...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_TRANSFORM));
	button.SetIcon(_iTransform);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Transform...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_SPAPER));
	button.SetIcon(_iSPaper);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Sand paper...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_FLIP));
	button.SetIcon(_iFlip);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "Flip...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

	button.Reset(GetDlgItem(IDC_CRV_CAGD));
	button.SetIcon(_iCAGD);
	ti.uId = (UINT) button.Hwnd();
	ti.lpszText = (LPSTR) "CAGD...";
	SendMessage(_hwndTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
	SendMessage(_hwndTT, TTM_ACTIVATE, true, 0);

#ifdef CURVEEDITRESIZABLE
	{
		RECT dlgrect;

		GetWindowRect(dlgrect);

		_ViewWnd.SetLeft(dlgrect);
		_ViewWnd.SetRight(dlgrect);
		_ViewWnd.SetTop(dlgrect);
		_ViewWnd.SetBottom(dlgrect);
		_ViewWnd.SetMinWidth();
		_ViewWnd.SetMinHeight();

		_Gallery.SetLeft(dlgrect);
		_Gallery.SetBottom(dlgrect);
		_Gallery.SetRight(dlgrect);
		_Gallery.SetHeight();
		_Gallery.SetMinWidth();
		_Gallery.SetMinTop(dlgrect);

		_NameLbl.SetRight(dlgrect);
		_NameLbl.SetBottom(dlgrect);
		_NameLbl.SetWidth();
		_NameLbl.SetHeight();
		_NameLbl.SetMinTop(dlgrect);
		_NameLbl.SetMinLeft(dlgrect);

		_Name.SetRight(dlgrect);
		_Name.SetBottom(dlgrect);
		_Name.SetWidth();
		_Name.SetHeight();
		_Name.SetMinTop(dlgrect);
		_Name.SetMinLeft(dlgrect);

		_StatWnd.SetLeft(dlgrect);
		_StatWnd.SetBottom(dlgrect);
		_StatWnd.SetRight(dlgrect);
		_StatWnd.SetHeight();
		_StatWnd.SetMinWidth();
		_StatWnd.SetMinTop(dlgrect);

		_gSimple.SetRight(dlgrect);
		_gSimple.SetTop(dlgrect);
		_gSimple.SetWidth();
		_gSimple.SetHeight();
		_gSimple.SetMinLeft(dlgrect);

		_gInitBSpline.SetRight(dlgrect);
		_gInitBSpline.SetTop(dlgrect);
		_gInitBSpline.SetWidth();
		_gInitBSpline.SetHeight();
		_gInitBSpline.SetMinLeft(dlgrect);

		_gInitBzSurface.SetRight(dlgrect);
		_gInitBzSurface.SetTop(dlgrect);
		_gInitBzSurface.SetWidth();
		_gInitBzSurface.SetHeight();
		_gInitBzSurface.SetMinLeft(dlgrect);

		_gOpen.SetRight(dlgrect);
		_gOpen.SetTop(dlgrect);
		_gOpen.SetWidth();
		_gOpen.SetHeight();
		_gOpen.SetMinLeft(dlgrect);

		_gSave.SetRight(dlgrect);
		_gSave.SetTop(dlgrect);
		_gSave.SetWidth();
		_gSave.SetHeight();
		_gSave.SetMinLeft(dlgrect);

		_gInitView.SetRight(dlgrect);
		_gInitView.SetTop(dlgrect);
		_gInitView.SetWidth();
		_gInitView.SetHeight();
		_gInitView.SetMinLeft(dlgrect);

		_gInitFit.SetRight(dlgrect);
		_gInitFit.SetTop(dlgrect);
		_gInitFit.SetWidth();
		_gInitFit.SetHeight();
		_gInitFit.SetMinLeft(dlgrect);

		_gUndo.SetRight(dlgrect);
		_gUndo.SetTop(dlgrect);
		_gUndo.SetWidth();
		_gUndo.SetHeight();
		_gUndo.SetMinLeft(dlgrect);

		_gRedo.SetRight(dlgrect);
		_gRedo.SetTop(dlgrect);
		_gRedo.SetWidth();
		_gRedo.SetHeight();
		_gRedo.SetMinLeft(dlgrect);

		_gPointer.SetRight(dlgrect);
		_gPointer.SetTop(dlgrect);
		_gPointer.SetWidth();
		_gPointer.SetHeight();
		_gPointer.SetMinLeft(dlgrect);

		_gRotate.SetRight(dlgrect);
		_gRotate.SetTop(dlgrect);
		_gRotate.SetWidth();
		_gRotate.SetHeight();
		_gRotate.SetMinLeft(dlgrect);

		_gScale.SetRight(dlgrect);
		_gScale.SetTop(dlgrect);
		_gScale.SetWidth();
		_gScale.SetHeight();
		_gScale.SetMinLeft(dlgrect);

		_gTranslate.SetRight(dlgrect);
		_gTranslate.SetTop(dlgrect);
		_gTranslate.SetWidth();
		_gTranslate.SetHeight();
		_gTranslate.SetMinLeft(dlgrect);

		_gAxes.SetRight(dlgrect);
		_gAxes.SetTop(dlgrect);
		_gAxes.SetWidth();
		_gAxes.SetHeight();
		_gAxes.SetMinLeft(dlgrect);

		_gGrid.SetRight(dlgrect);
		_gGrid.SetTop(dlgrect);
		_gGrid.SetWidth();
		_gGrid.SetHeight();
		_gGrid.SetMinLeft(dlgrect);

		_gInsert.SetRight(dlgrect);
		_gInsert.SetTop(dlgrect);
		_gInsert.SetWidth();
		_gInsert.SetHeight();
		_gInsert.SetMinLeft(dlgrect);

		_gDelete.SetRight(dlgrect);
		_gDelete.SetTop(dlgrect);
		_gDelete.SetWidth();
		_gDelete.SetHeight();
		_gDelete.SetMinLeft(dlgrect);

		_gMult.SetRight(dlgrect);
		_gMult.SetTop(dlgrect);
		_gMult.SetWidth();
		_gMult.SetHeight();
		_gMult.SetMinLeft(dlgrect);

		_gType.SetRight(dlgrect);
		_gType.SetTop(dlgrect);
		_gType.SetWidth();
		_gType.SetHeight();
		_gType.SetMinLeft(dlgrect);

		_gSelect.SetRight(dlgrect);
		_gSelect.SetTop(dlgrect);
		_gSelect.SetWidth();
		_gSelect.SetHeight();
		_gSelect.SetMinLeft(dlgrect);

		_gAlign.SetRight(dlgrect);
		_gAlign.SetTop(dlgrect);
		_gAlign.SetWidth();
		_gAlign.SetHeight();
		_gAlign.SetMinLeft(dlgrect);

		_gDPoints.SetRight(dlgrect);
		_gDPoints.SetTop(dlgrect);
		_gDPoints.SetWidth();
		_gDPoints.SetHeight();
		_gDPoints.SetMinLeft(dlgrect);

		_gDConvex.SetRight(dlgrect);
		_gDConvex.SetTop(dlgrect);
		_gDConvex.SetWidth();
		_gDConvex.SetHeight();
		_gDConvex.SetMinLeft(dlgrect);

		_gDCurve.SetRight(dlgrect);
		_gDCurve.SetTop(dlgrect);
		_gDCurve.SetWidth();
		_gDCurve.SetHeight();
		_gDCurve.SetMinLeft(dlgrect);

		_gDivide.SetRight(dlgrect);
		_gDivide.SetTop(dlgrect);
		_gDivide.SetWidth();
		_gDivide.SetHeight();
		_gDivide.SetMinLeft(dlgrect);

		_gShow.SetRight(dlgrect);
		_gShow.SetTop(dlgrect);
		_gShow.SetWidth();
		_gShow.SetHeight();
		_gShow.SetMinLeft(dlgrect);

		_gHide.SetRight(dlgrect);
		_gHide.SetTop(dlgrect);
		_gHide.SetWidth();
		_gHide.SetHeight();
		_gHide.SetMinLeft(dlgrect);

		_gBright.SetRight(dlgrect);
		_gBright.SetTop(dlgrect);
		_gBright.SetWidth();
		_gBright.SetHeight();
		_gBright.SetMinLeft(dlgrect);

		_gDark.SetRight(dlgrect);
		_gDark.SetTop(dlgrect);
		_gDark.SetWidth();
		_gDark.SetHeight();
		_gDark.SetMinLeft(dlgrect);

		_gTurtle.SetRight(dlgrect);
		_gTurtle.SetTop(dlgrect);
		_gTurtle.SetWidth();
		_gTurtle.SetHeight();
		_gTurtle.SetMinLeft(dlgrect);

		_gTransform.SetRight(dlgrect);
		_gTransform.SetTop(dlgrect);
		_gTransform.SetWidth();
		_gTransform.SetHeight();
		_gTransform.SetMinLeft(dlgrect);

		_gSPaper.SetRight(dlgrect);
		_gSPaper.SetTop(dlgrect);
		_gSPaper.SetWidth();
		_gSPaper.SetHeight();
		_gSPaper.SetMinLeft(dlgrect);

		_gFlip.SetRight(dlgrect);
		_gFlip.SetTop(dlgrect);
		_gFlip.SetWidth();
		_gFlip.SetHeight();
		_gFlip.SetMinLeft(dlgrect);

		_gCAGD.SetRight(dlgrect);
		_gCAGD.SetTop(dlgrect);
		_gCAGD.SetWidth();
		_gCAGD.SetHeight();
		_gCAGD.SetMinLeft(dlgrect);
	}
#endif

	updateTool();

	_rotA = _rotX = _rotY =_rotZ = 0;
	_scaleX = _scaleY = _scaleZ = 1;
	_transX = _transY = _transZ = 0;
	_setX = _setY =_setZ = 0;

	_crvA = _crvB = _crvC = _crvD = 0;
	_crvF = 0;
	_crvT = 1;
	_crvN = 4;
}


CurveEdit::~CurveEdit()
{
	DestroyWindow(_hwndTT);
}


HWND CurveEdit::Create(HWND hParent, HINSTANCE hInst, PrjNotifySink* pNotifySink)
{
	return CreateDialogParam
		(
		hInst, 
		MAKEINTRESOURCE(IDD_CURVES),
		hParent,
		reinterpret_cast<DLGPROC>(CurveEdit::DlgProc),
		reinterpret_cast<LPARAM>(pNotifySink)
		);
}


BOOL CALLBACK CurveEdit::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CurveEdit* pCtrl = GetWinLong<CurveEdit*>(hDlg);
	switch (msg)
	{
	case WM_INITDIALOG :
		{
			try
			{
				pCtrl = new CurveEdit(hDlg, GetWindowInstance(hDlg), reinterpret_cast<PrjNotifySink*>(lParam));
			}
			catch (Exception e)
			{
				::MessageBox(hDlg, e.Msg(), "Error", MB_ICONSTOP);
				EndDialog(hDlg, 0);
			}
		}
		return true;
	case WM_DESTROY :
		CurveEdit::OnDestroy(hDlg);
		return true;
	case WM_COMMAND :
		return pCtrl->Command(static_cast<int>(LOWORD(wParam)), Window(reinterpret_cast<HWND>(lParam)), static_cast<UINT>(HIWORD(wParam)));
#ifdef CURVEEDITRESIZABLE
	case WM_SIZE:
		return pCtrl->Size(SizeState(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
#endif
	}
	return false;
}

enum
{
	IDP_CRV_SELECT_P0 = 40100,
		IDP_CRV_SELECT_P1,
		IDP_CRV_SELECT_P2,
		IDP_CRV_SELECT_P3,
		IDP_CRV_SELECT_P4,
		IDP_CRV_SELECT_P5,
		IDP_CRV_SELECT_P6,
		IDP_CRV_SELECT_P7
};



bool CurveEdit::Command(int id, Window ctl, UINT)
{
	RECT r;

	switch (id)
	{
	case IDC_SIMPLE :
		_Simple();
		break;
	case IDC_CRV_INIT_BSPLINE :
		mInitBSpline();
		break;
	case IDC_CRV_INIT_BZSURFACE :
		mInitBzSurface();
		break;
	case IDC_CRV_OPEN :
		mOpen();
		break;
	case IDC_CRV_SAVE :
		mSave();
		break;
	case IDC_CRV_INIT_VIEW :
		mInitView();
		break;
	case IDC_CRV_INIT_FIT :
		mInitFit();
		break;
	case IDC_CRV_UNDO :
		mUndo();
		break;
	case IDC_CRV_REDO :
		mRedo();
		break;
	case IDC_CRV_POINTER :
		mPointer();
		break;
	case IDC_CRV_ROTATE :
		mRotate();
		break;
	case IDC_CRV_SCALE :
		mScale();
		break;
	case IDC_CRV_TRANSLATE :
		mTranslate();
		break;
	case IDC_CRV_AXES :
		mAxes();
		break;
	case IDC_CRV_GRID :
		ctl.GetWindowRect(r);
		mGrid(r.left+8, r.top+8);
		break;
	case IDP_CRV_GRID_GRID :
		mGridGrid();
		break;
	case IDP_CRV_GRID_XY :
		mGridXY();
		break;
	case IDP_CRV_GRID_XZ :
		mGridXZ();
		break;
	case IDP_CRV_GRID_YZ :
		mGridYZ();
		break;
	case IDC_CRV_INSERT :
		ctl.GetWindowRect(r);
		mInsert(r.left+8, r.top+8);
		break;
	case IDP_CRV_IPOINT_PRED :
		mAddPred();
		break;
	case IDP_CRV_IPOINT_SUCC :
		mAddSucc();
		break;
	case IDP_CRV_ISURFACE_ABOVE_LEFT :
		mAddNW();
		break;
	case IDP_CRV_ISURFACE_ABOVE :
		mAddN();
		break;
	case IDP_CRV_ISURFACE_ABOVE_RIGHT :
		mAddNE();
		break;
	case IDP_CRV_ISURFACE_LEFT:
		mAddW();
		break;
	case IDP_CRV_ISURFACE_RIGHT :
		mAddE();
		break;
	case IDP_CRV_ISURFACE_BELOW_LEFT:
		mAddSW();
		break;
	case IDP_CRV_ISURFACE_BELOW :
		mAddS();
		break;
	case IDP_CRV_ISURFACE_BELOW_RIGHT :
		mAddSE();
		break;
	case IDP_CRV_IVARIATION_BSPLINE :
		mVarBSpline();
		break;
	case IDP_CRV_IVARIATION_BZSURFACE :
		mVarBzSurface();
		break;
	case IDC_CRV_DELETE :
		ctl.GetWindowRect(r);
		mDelete(r.left+8, r.top+8);
		break;
	case IDP_CRV_DELETE_POINT :
		mDelPoint();
		break;
	case IDP_CRV_DELETE_SURFACE :
		mDelSurface();
		break;
	case IDP_CRV_DELETE_VARIATION :
		mDelVariation();
		break;
	case IDC_CRV_MULT :
		ctl.GetWindowRect(r);
		mMult(r.left+8, r.top+8);
		break;
	case IDP_CRV_MULT_1 :
		mMult1();
		break;
	case IDP_CRV_MULT_2 :
		mMult2();
		break;
	case IDP_CRV_MULT_3 :
		mMult3();
		break;
	case IDP_CRV_MULT_4 :
		mMult4();
		break;
	case IDC_CRV_TYPE :
		ctl.GetWindowRect(r);
		mType(r.left+8, r.top+8);
		break;
	case IDP_CRV_TYPE_BSPLINE :
		mTypeBSpline();
		break;
	case IDP_CRV_TYPE_BCLOSED :
		mTypeBClosed();
		break;
	case IDP_CRV_TYPE_BENDPOINT :
		mTypeBEndpoint();
		break;
	case IDP_CRV_TYPE_BPHANTOM :
		mTypeBPhantom();
		break;
	case IDP_CRV_TYPE_BZSPLINE :
		mTypeBzSpline();
		break;
	case IDP_CRV_TYPE_BZSURFACE :
		mTypeBzSurface();
		break;
	case IDC_CRV_SELECT :
		ctl.GetWindowRect(r);
		mSelect(r.left+8, r.top+8);
		break;		
	case IDP_CRV_SELECT_ALL :
		mSelectAll();
		break;
	case IDP_CRV_SELECT_CURVE :
		mSelectCurve();
		break;
	case IDP_CRV_SELECT_P0 :
    case IDP_CRV_SELECT_P1 :
	case IDP_CRV_SELECT_P2 :
	case IDP_CRV_SELECT_P3 :
    case IDP_CRV_SELECT_P4 :
	case IDP_CRV_SELECT_P5 :
		mSelectPX(id-IDP_CRV_SELECT_P0);
		break;
	case IDC_CRV_ALIGN :
		mAlign();
		break;
	case IDC_CRV_DPOINTS :
		mDPoints();
		break;
	case IDC_CRV_DCONVEX :
		mDConvex();
		break;
	case IDC_CRV_DCURVE :
		mDCurve();
		break;
	case IDC_CRV_DIVIDE :
		mDivide();
		break;
	case IDC_CRV_SHOW :
		ctl.GetWindowRect(r);
		mShow(r.left+8, r.top+8);
		break;
	case IDP_CRV_SHOW_ALL :
		mShowAll();
		break;
	case IDP_CRV_SHOW_CURVE :
		mShowCurve();
		break;
	case IDP_CRV_SHOW_PATCH :
		mShowPatch();
		break;
	case IDP_CRV_SHOW_POINT :
		mShowPoint();
		break;
	case IDC_CRV_HIDE :
		ctl.GetWindowRect(r);
		mHide(r.left+8, r.top+8);
		break;
	case IDP_CRV_HIDE_CURVE :
		mHideCurve();
		break;
	case IDP_CRV_HIDE_PATCH :
		mHidePatch();
		break;
	case IDP_CRV_HIDE_POINT :
		mHidePoint();
		break;
	case IDC_CRV_BRIGHT :
		ctl.GetWindowRect(r);
		mBright(r.left+8, r.top+8);
		break;
	case IDP_CRV_BRIGHT_ALL :
		mBrightAll();
		break;
	case IDP_CRV_BRIGHT_CURVE :
		mBrightCurve();
		break;
	case IDP_CRV_BRIGHT_PATCH :
		mBrightPatch();
		break;
	case IDP_CRV_BRIGHT_POINT :
		mBrightPoint();
		break;
	case IDC_CRV_DARK :
		ctl.GetWindowRect(r);
		mDark(r.left+8, r.top+8);
		break;
	case IDP_CRV_DARK_CURVE :
		mDarkCurve();
		break;
	case IDP_CRV_DARK_PATCH :
		mDarkPatch();
		break;
	case IDP_CRV_DARK_POINT :
		mDarkPoint();
		break;
	case IDC_CRV_TURTLE :
		mTurtle();
		break;
	case IDC_CRV_TRANSFORM :
		mTransform();
		break;
	case IDC_CRV_SPAPER :
		ctl.GetWindowRect(r);
		mSPaper(r.left+8, r.top+8);
		break;
	case IDP_CRV_SPATCH_ALL :
		mSPatchAll();
		break;
	case IDP_CRV_SPATCH_CORNER :
		mSPatchCorner();
		break;
	case IDP_CRV_SPATCH_LEFT :
		mSPatchLeft();
		break;
	case IDP_CRV_SPATCH_RIGHT :
		mSPatchRight();
		break;
	case IDP_CRV_SPATCH_TOP :
		mSPatchTop();
		break;
	case IDP_CRV_SPATCH_BOTTOM :
		mSPatchBottom();
		break;
	case IDC_CRV_FLIP :
		ctl.GetWindowRect(r);
		mFlip(r.left+8, r.top+8);
		break;
	case IDP_CRV_FPATCH_X :
		mFPatchX();
		break;
	case IDP_CRV_FPATCH_Y :
		mFPatchY();
		break;
	case IDC_CRV_CAGD :
		ctl.GetWindowRect(r);
		mCAGD(r.left+8, r.top+8);
		break;
	case IDP_CRV_CAGD_BLOSSOM :
		mCBlossom();
		break;
	case IDP_CRV_CAGD_INTERPOLATION :
		mCInterpolation();
		break;
	}
	return true;
}


void CurveEdit::Import(const TCHAR* fname)
{
	CurveXYZView* pNew = new CurveXYZView;

	std::unique_ptr<EditableObject> New(pNew);

	const char* pDot = strrchr(fname, '.');
	if ( 0 != pDot )
	{
		if ( (_stricmp(pDot, ".crv-cfg") == 0) )
		{
			_curveXYZView->openConfig(fname);
		}
		else
		{
		    pNew->openFile(fname);
			pNew->fitPoints();

			_pGallery->Add(New);
		}
	}
}


void CurveEdit::Generate() const
{
	int i;

	for (i = 0; i < _pGallery->Items(); i++)
	{
		const CurveXYZView* pCurve = dynamic_cast<const CurveXYZView*>(_pGallery->GetObject(i));

		pCurve->saveFile(pCurve->_filename);
	}
}


#ifdef CURVEEDITRESIZABLE
bool CurveEdit::Size(SizeState, int w, int h)
{
	_ViewWnd.Adjust(w, h);
	_Gallery.Adjust(w, h);
	_NameLbl.Adjust(w, h);
	_Name.Adjust(w, h);
	_StatWnd.Adjust(w, h);
	_gSimple.Adjust(w, h);
	_gInitBSpline.Adjust(w, h);
	_gInitBzSurface.Adjust(w, h);
	_gOpen.Adjust(w, h);
	_gSave.Adjust(w, h);
	_gInitView.Adjust(w, h);
	_gInitFit.Adjust(w, h);
	_gUndo.Adjust(w, h);
	_gRedo.Adjust(w, h);
	_gPointer.Adjust(w, h);
	_gRotate.Adjust(w, h);
	_gScale.Adjust(w, h);
	_gTranslate.Adjust(w, h);
	_gAxes.Adjust(w, h);
	_gGrid.Adjust(w, h);
	_gInsert.Adjust(w, h);
	_gDelete.Adjust(w, h);
	_gMult.Adjust(w, h);
	_gType.Adjust(w, h);
	_gSelect.Adjust(w, h);
	_gAlign.Adjust(w, h);
	_gDPoints.Adjust(w, h);
	_gDConvex.Adjust(w, h);
	_gDCurve.Adjust(w, h);
	_gDivide.Adjust(w, h);
	_gShow.Adjust(w, h);
	_gHide.Adjust(w, h);
	_gBright.Adjust(w, h);
	_gDark.Adjust(w, h);
	_gTurtle.Adjust(w, h);
	_gTransform.Adjust(w, h);
	_gSPaper.Adjust(w, h);
	_gFlip.Adjust(w, h);
	_gCAGD.Adjust(w, h);
	return true;
}
#endif


void CurveEdit::_UpdateControls()
{
	updateTool();
}


void CurveEdit::_UpdateFromControls()
{
	EditLine name(GetDlgItem(IDC_NAME));

	std::string buf;
	name.GetText(buf);

	strcpy(_curveXYZView->_filename, buf.c_str());
}


void CurveEdit::updateEdit()
{
	updateTool();
	updateMenu();
	updateStat();
	Modified();
}



void CurveEdit::createSelect(CCurve *curve, int level)
{
	int  i;
	char buf[256];

	if ( curve != 0 )
	{
		HMENU hMenu = App::theApp->GetContextMenu(CurveSelectCMenu);

		MENUITEMINFO menuInfo;
		menuInfo.cbSize = sizeof(MENUITEMINFO);
		menuInfo.fMask = (MIIM_CHECKMARKS | MIIM_DATA | 
			MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE);
		menuInfo.fType = MFT_STRING;
		menuInfo.fState = MFS_UNHILITE;
		menuInfo.hSubMenu = 0;
		menuInfo.hbmpChecked = 0;
		menuInfo.hbmpUnchecked = 0;
		menuInfo.dwItemData = 0;

		for (i = 0; i < curve->_csize; i++)
		{
			if ( curve->_point[i]._select > 0 )
			{
				if ( _selects < 8 )
				{
					if ( curve->_ctype != eBEZIERSURFACE )
						sprintf(buf, "Point %d %d", level, i);
					else
						sprintf(buf, "Point %d %d %d", level, i/16+1, i%16+1);

					menuInfo.wID = IDP_CRV_SELECT_P0+_selects;
					menuInfo.dwTypeData = buf;

					InsertMenuItem(hMenu, _selects+2, true, &menuInfo);

					_selects += 1;
				}
			}

			createSelect(curve->_point[i]._curve, level+1);
		}
	}
}

void CurveEdit::updateMenu()
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveSelectCMenu);

	while ( GetMenuItemCount(hMenu) > 2 )
		DeleteMenu(hMenu, 2, MF_BYPOSITION);

	_selects = 0;

	createSelect(_curveXYZView->_curveXYZ->_curve, 0);
}


void CurveEdit::updateTool()
{

	{
		Button button(GetDlgItem(IDC_CRV_POINTER));
		button.SetCheck(_curveXYZView->_pointer);

		button.Reset(GetDlgItem(IDC_CRV_ROTATE));
		button.SetCheck(_curveXYZView->_rotTB);

		button.Reset(GetDlgItem(IDC_CRV_SCALE));
		button.SetCheck(_curveXYZView->_scaleTB);

		button.Reset(GetDlgItem(IDC_CRV_TRANSLATE));
		button.SetCheck(_curveXYZView->_transTB);

		button.Reset(GetDlgItem(IDC_CRV_AXES));
		button.SetCheck(_curveXYZView->_axes);
	}

	{
		EditLine name(GetDlgItem(IDC_NAME));
		name.SetText(_curveXYZView->_filename);
	}
}


void CurveEdit::updateStat()
{
	char buf[256];

	if ( _curveXYZView->_selectc != 0 )
	{
		if ( _curveXYZView->_selectc->_ctype != eBEZIERSURFACE )
		{
			sprintf(buf, "L: %d P: %d x: %.2f y: %.2f z: %.2f m: %d",
				_curveXYZView->_selectl, _curveXYZView->_selectp,
				_curveXYZView->_selectc->_point[_curveXYZView->_selectp]._point.x,
				_curveXYZView->_selectc->_point[_curveXYZView->_selectp]._point.y,
				_curveXYZView->_selectc->_point[_curveXYZView->_selectp]._point.z,
				_curveXYZView->_selectc->_point[_curveXYZView->_selectp]._pmult);
		}
		else
		{
			sprintf(buf, "L: %d P: %d %d x: %.2f y: %.2f z: %.2f",
				_curveXYZView->_selectl, 
				_curveXYZView->_selectp/16+1, _curveXYZView->_selectp%16+1,
				_curveXYZView->_selectc->_point[_curveXYZView->_selectp]._point.x,
				_curveXYZView->_selectc->_point[_curveXYZView->_selectp]._point.y,
				_curveXYZView->_selectc->_point[_curveXYZView->_selectp]._point.z);
		}

		updateStat(buf);
	}
}


void CurveEdit::updateStat(const char* buf)
{
	Static label(GetDlgItem(IDC_CRV_STATUS));
	label.SetText(buf);
}


void CurveEdit::_Simple()
{
	_pNotifySink->SimpleSurfaceMode();
}


void CurveEdit::mInitBSpline()
{
	_curveXYZView->mInitBSpline();
}


void CurveEdit::mInitBzSurface()
{
	_curveXYZView->mInitBzSurface();
}


void CurveEdit::mOpen()
{
	OpenFilename ofn(Hwnd(), IDS_CURVEFILTER, __TEXT(""));
	ofn.Flags |= OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
	if (ofn.Open())
	{
		_curveXYZView->mOpen(ofn.Filename());
	}
}


void CurveEdit::mSave()
{
	OpenFilename ofn(Hwnd(), IDS_CURVEFILTER, __TEXT(""));
	if (ofn.Save())
	{
		_curveXYZView->mSave(ofn.Filename());
	}
}


void CurveEdit::mUndo()
{
	_curveXYZView->undoUndo();
}


void CurveEdit::mRedo()
{
	_curveXYZView->undoRedo();
}


void CurveEdit::mInitView()
{
	_curveXYZView->mInitView();
}


void CurveEdit::mInitFit()
{
	_curveXYZView->mInitFit();
}


void CurveEdit::mPointer()
{
	_curveXYZView->mPointer();
}


void CurveEdit::mRotate()
{
	_curveXYZView->mRotate();
}


void CurveEdit::mScale()
{
	_curveXYZView->mScale();
}


void CurveEdit::mTranslate()
{
	_curveXYZView->mTranslate();
}


void CurveEdit::mAxes()
{
	_curveXYZView->mAxes();
}


void CurveEdit::mGrid(int x, int y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveGridCMenu);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, Hwnd(), 0);
}


void CurveEdit::mGridGrid()
{
	_curveXYZView->mGridGrid();
}


void CurveEdit::mGridXY()
{
	_curveXYZView->mGridXY();
}


void CurveEdit::mGridXZ()
{
	_curveXYZView->mGridXZ();
}


void CurveEdit::mGridYZ()
{
	_curveXYZView->mGridYZ();
}


void CurveEdit::mInsert(int x, int y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveInsertCMenu);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, Hwnd(), 0);
}


void CurveEdit::mAddPred()
{
	_curveXYZView->mAddPred();
}


void CurveEdit::mAddSucc()
{
	_curveXYZView->mAddSucc();
}


void CurveEdit::mAddN()
{
	_curveXYZView->mAddN();
}


void CurveEdit::mAddNE()
{
	_curveXYZView->mAddNE();
}


void CurveEdit::mAddE()
{
	_curveXYZView->mAddE();
}


void CurveEdit::mAddSE()
{
	_curveXYZView->mAddSE();
}


void CurveEdit::mAddS()
{
	_curveXYZView->mAddS();
}


void CurveEdit::mAddSW()
{
	_curveXYZView->mAddSW();
}


void CurveEdit::mAddW()
{
	_curveXYZView->mAddW();
}


void CurveEdit::mAddNW()
{
	_curveXYZView->mAddNW();
}


void CurveEdit::mVarBSpline()
{
	_curveXYZView->mVarBSpline();
}


void CurveEdit::mVarBzSurface()
{
	_curveXYZView->mVarBzSurface();
}


void CurveEdit::mDelete(int x, int y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveDeleteCMenu);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, Hwnd(), 0);
}


void CurveEdit::mDelPoint()
{
	_curveXYZView->mDelPoint();
}


void CurveEdit::mDelSurface()
{
	_curveXYZView->mDelSurface();
}


void CurveEdit::mDelVariation()
{
	_curveXYZView->mVarDel();
}


void CurveEdit::mMult(int x, int y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveMultCMenu);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, Hwnd(), 0);
}


void CurveEdit::mMult1()
{
	_curveXYZView->mMult1();
}


void CurveEdit::mMult2()
{
	_curveXYZView->mMult2();
}


void CurveEdit::mMult3()
{
	_curveXYZView->mMult3();
}


void CurveEdit::mMult4()
{
	_curveXYZView->mMult4();
}


void CurveEdit::mType(int x, int y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveTypeCMenu);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, Hwnd(), 0);
}


void CurveEdit::mTypeBSpline()
{
	_curveXYZView->mTypeBSpline();
}


void CurveEdit::mTypeBClosed()
{
	_curveXYZView->mTypeBClosed();
}


void CurveEdit::mTypeBEndpoint()
{
	_curveXYZView->mTypeBEndpoint();
}


void CurveEdit::mTypeBPhantom()
{
	_curveXYZView->mTypeBPhantom();
}


void CurveEdit::mTypeBzSpline()
{
	_curveXYZView->mTypeBzSpline();
}


void CurveEdit::mTypeBzSurface()
{
	_curveXYZView->mTypeBzSurface();
}


void CurveEdit::mSelect(int x, int y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveSelectCMenu);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, Hwnd(), 0);
}


void CurveEdit::mSelectAll()
{
	_curveXYZView->mSelectAll();
}


void CurveEdit::mSelectCurve()
{
	_curveXYZView->mSelectCurve();
}


void CurveEdit::mSelectPX(int p)
{
	_curveXYZView->mSelectPX(p);
}


void CurveEdit::mAlign()
{
	_curveXYZView->mAlign();
}


void CurveEdit::mDPoints()
{
	_curveXYZView->mDPoints();
}


void CurveEdit::mDConvex()
{
	_curveXYZView->mDConvex();
}


void CurveEdit::mDCurve()
{
	_curveXYZView->mDCurve();
}


void CurveEdit::mDivide()
{
	_curveXYZView->mDivide();
}


void CurveEdit::mShow(int x, int y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveShowCMenu);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, Hwnd(), 0);
}


void CurveEdit::mShowAll()
{
	_curveXYZView->mShowAll();
}


void CurveEdit::mShowCurve()
{
	_curveXYZView->mShowCurve();
}


void CurveEdit::mShowPatch()
{
	_curveXYZView->mShowPatch();
}


void CurveEdit::mShowPoint()
{
	_curveXYZView->mShowPoint();
}


void CurveEdit::mHide(int x, int y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveHideCMenu);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, Hwnd(), 0);
}


void CurveEdit::mHideCurve()
{
	_curveXYZView->mHideCurve();
}


void CurveEdit::mHidePatch()
{
	_curveXYZView->mHidePatch();
}


void CurveEdit::mHidePoint()
{
	_curveXYZView->mHidePoint();
}


void CurveEdit::mBright(int x, int y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveBrightCMenu);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, Hwnd(), 0);
}


void CurveEdit::mBrightAll()
{
	_curveXYZView->mBrightAll();
}


void CurveEdit::mBrightCurve()
{
	_curveXYZView->mBrightCurve();
}


void CurveEdit::mBrightPatch()
{
	_curveXYZView->mBrightPatch();
}


void CurveEdit::mBrightPoint()
{
	_curveXYZView->mBrightPoint();
}


void CurveEdit::mDark(int x, int y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveDarkCMenu);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, Hwnd(), 0);
}


void CurveEdit::mDarkCurve()
{
	_curveXYZView->mDarkCurve();
}


void CurveEdit::mDarkPatch()
{
	_curveXYZView->mDarkPatch();
}


void CurveEdit::mDarkPoint()
{
	_curveXYZView->mDarkPoint();
}


void CurveEdit::mTurtle()
{
	CurveTurtleDlg dlg;

	sprintf(dlg._contactX.Buf(), "%.2f", _curveXYZView->_curveXYZ->_turtle._cp.x);
	dlg._contactX.CalcLength();
	sprintf(dlg._contactY.Buf(), "%.2f", _curveXYZView->_curveXYZ->_turtle._cp.y);
	dlg._contactY.CalcLength();
	sprintf(dlg._contactZ.Buf(), "%.2f", _curveXYZView->_curveXYZ->_turtle._cp.z);
	dlg._contactZ.CalcLength();

	sprintf(dlg._endX.Buf(), "%.2f", _curveXYZView->_curveXYZ->_turtle._ep.x);
	dlg._endX.CalcLength();
	sprintf(dlg._endY.Buf(), "%.2f", _curveXYZView->_curveXYZ->_turtle._ep.y);
	dlg._endY.CalcLength();
	sprintf(dlg._endZ.Buf(), "%.2f", _curveXYZView->_curveXYZ->_turtle._ep.z);
	dlg._endZ.CalcLength();

	sprintf(dlg._headX.Buf(), "%.2f", _curveXYZView->_curveXYZ->_turtle._h.x);
	dlg._headX.CalcLength();
	sprintf(dlg._headY.Buf(), "%.2f", _curveXYZView->_curveXYZ->_turtle._h.y);
	dlg._headY.CalcLength();
	sprintf(dlg._headZ.Buf(), "%.2f", _curveXYZView->_curveXYZ->_turtle._h.z);
	dlg._headZ.CalcLength();

	sprintf(dlg._upX.Buf(), "%.2f", _curveXYZView->_curveXYZ->_turtle._u.x);
	dlg._upX.CalcLength();
	sprintf(dlg._upY.Buf(), "%.2f", _curveXYZView->_curveXYZ->_turtle._u.y);
	dlg._upY.CalcLength();
	sprintf(dlg._upZ.Buf(), "%.2f", _curveXYZView->_curveXYZ->_turtle._u.z);
	dlg._upZ.CalcLength();

	sprintf(dlg._size.Buf(), "%.2f", _curveXYZView->_curveXYZ->_turtle._sz);
	dlg._size.CalcLength();

	if ( dlg.DoModal(*this) == IDOK )
	{
		_curveXYZView->_curveXYZ->_turtle._cp.x = static_cast<float>(atof(dlg._contactX));
		_curveXYZView->_curveXYZ->_turtle._cp.y = static_cast<float>(atof(dlg._contactY));
		_curveXYZView->_curveXYZ->_turtle._cp.z = static_cast<float>(atof(dlg._contactZ));

		_curveXYZView->_curveXYZ->_turtle._ep.x = static_cast<float>(atof(dlg._endX));
		_curveXYZView->_curveXYZ->_turtle._ep.y = static_cast<float>(atof(dlg._endY));
		_curveXYZView->_curveXYZ->_turtle._ep.z = static_cast<float>(atof(dlg._endZ));

		_curveXYZView->_curveXYZ->_turtle._h.x = static_cast<float>(atof(dlg._headX));
		_curveXYZView->_curveXYZ->_turtle._h.y = static_cast<float>(atof(dlg._headY));
		_curveXYZView->_curveXYZ->_turtle._h.z = static_cast<float>(atof(dlg._headZ));

		_curveXYZView->_curveXYZ->_turtle._u.x = static_cast<float>(atof(dlg._upX));
		_curveXYZView->_curveXYZ->_turtle._u.y = static_cast<float>(atof(dlg._upY));
		_curveXYZView->_curveXYZ->_turtle._u.z = static_cast<float>(atof(dlg._upZ));

		_curveXYZView->_curveXYZ->_turtle._sz = static_cast<float>(atof(dlg._size));
	}
}


void CurveEdit::mTransform()
{
	int               r;
	CurveTransformDlg dlg;

	sprintf(dlg._rotA.Buf(), "%.2f", _rotA);
	dlg._rotA.CalcLength();
	sprintf(dlg._rotX.Buf(), "%.2f", _rotX);
	dlg._rotX.CalcLength();
	sprintf(dlg._rotY.Buf(), "%.2f", _rotY);
	dlg._rotY.CalcLength();
	sprintf(dlg._rotZ.Buf(), "%.2f", _rotZ);
	dlg._rotZ.CalcLength();

	sprintf(dlg._scaleX.Buf(), "%.2f", _scaleX);
	dlg._scaleX.CalcLength();
	sprintf(dlg._scaleY.Buf(), "%.2f", _scaleY);
	dlg._scaleY.CalcLength();
	sprintf(dlg._scaleZ.Buf(), "%.2f", _scaleZ);
	dlg._scaleZ.CalcLength();

	sprintf(dlg._transX.Buf(), "%.2f", _transX);
	dlg._transX.CalcLength();
	sprintf(dlg._transY.Buf(), "%.2f", _transY);
	dlg._transY.CalcLength();
	sprintf(dlg._transZ.Buf(), "%.2f", _transZ);
	dlg._transZ.CalcLength();

	sprintf(dlg._setX.Buf(), "%.2f", _setX);
	dlg._setX.CalcLength();
	sprintf(dlg._setY.Buf(), "%.2f", _setY);
	dlg._setY.CalcLength();
	sprintf(dlg._setZ.Buf(), "%.2f", _setZ);
	dlg._setZ.CalcLength();

	r = dlg.DoModal(*this);
	if ( (r == IDB_CRV_TRANSFORM_R) || (r == IDB_CRV_TRANSFORM_S) || 
		 (r == IDB_CRV_TRANSFORM_T) || (r == IDB_CRV_TRANSFORM_E) )
	{
		_rotA = static_cast<float>(atof(dlg._rotA));
		_rotX = static_cast<float>(atof(dlg._rotX));
		_rotY = static_cast<float>(atof(dlg._rotY));
		_rotZ = static_cast<float>(atof(dlg._rotZ));

		_scaleX = static_cast<float>(atof(dlg._scaleX));
		_scaleY = static_cast<float>(atof(dlg._scaleY));
		_scaleZ = static_cast<float>(atof(dlg._scaleZ));

		_transX = static_cast<float>(atof(dlg._transX));
		_transY = static_cast<float>(atof(dlg._transY));
		_transZ = static_cast<float>(atof(dlg._transZ));

		_setX = static_cast<float>(atof(dlg._setX));
		_setY = static_cast<float>(atof(dlg._setY));
		_setZ = static_cast<float>(atof(dlg._setZ));

		if ( r == IDB_CRV_TRANSFORM_R )
			_curveXYZView->mRotate(_rotA, _rotX, _rotY, _rotZ);
		if ( r == IDB_CRV_TRANSFORM_S )
			_curveXYZView->mScale(_scaleX, _scaleY, _scaleZ);
		if ( r == IDB_CRV_TRANSFORM_T )
			_curveXYZView->mTranslate(_transX, _transY, _transZ);
		if ( r == IDB_CRV_TRANSFORM_E )
			_curveXYZView->mSet(_setX, _setY, _setZ);
	}
}


void CurveEdit::mSPaper(int x, int y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveSPaperCMenu);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, Hwnd(), 0);
}


void CurveEdit::mSPatchAll()
{
	_curveXYZView->mSPatchAll();
}


void CurveEdit::mSPatchCorner()
{
	_curveXYZView->mSPatchCorner();
}


void CurveEdit::mSPatchLeft()
{
	_curveXYZView->mSPatchLeft();
}


void CurveEdit::mSPatchRight()
{
	_curveXYZView->mSPatchRight();
}


void CurveEdit::mSPatchTop()
{
	_curveXYZView->mSPatchTop();
}


void CurveEdit::mSPatchBottom()
{
	_curveXYZView->mSPatchBottom();
}


void CurveEdit::mFlip(int x, int y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveFlipCMenu);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, Hwnd(), 0);
}


void CurveEdit::mFPatchX()
{
	_curveXYZView->mFPatchX();
}


void CurveEdit::mFPatchY()
{
	_curveXYZView->mFPatchY();
}


void CurveEdit::mCAGD(int x, int y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveCAGDCMenu);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, Hwnd(), 0);
}


void CurveEdit::mCBlossom()
{
	CurveCAGDDlg dlg;

	sprintf(dlg._crvA.Buf(), "%.5f", _crvA);
	dlg._crvA.CalcLength();
	sprintf(dlg._crvB.Buf(), "%.5f", _crvB);
	dlg._crvB.CalcLength();
	sprintf(dlg._crvC.Buf(), "%.5f", _crvC);
	dlg._crvC.CalcLength();
	sprintf(dlg._crvD.Buf(), "%.5f", _crvD);
	dlg._crvD.CalcLength();

	sprintf(dlg._crvF.Buf(), "%.5f", _crvF);
	dlg._crvF.CalcLength();

	sprintf(dlg._crvT.Buf(), "%.5f", _crvT);
	dlg._crvT.CalcLength();

	sprintf(dlg._crvN.Buf(), "%i", _crvN);
	dlg._crvN.CalcLength();


	if ( dlg.DoModal(*this) == IDOK )
	{
		_crvA = static_cast<float>(atof(dlg._crvA));
		_crvB = static_cast<float>(atof(dlg._crvB));
		_crvC = static_cast<float>(atof(dlg._crvC));
		_crvD = static_cast<float>(atof(dlg._crvD));

		_crvF = static_cast<float>(atof(dlg._crvF));

		_crvT = static_cast<float>(atof(dlg._crvT));

		_crvN = (int)atof(dlg._crvN);

		_curveXYZView->mCBlossom(_crvA, _crvB, _crvC, _crvD, _crvF, _crvT);
	}
}


void CurveEdit::mCInterpolation()
{
	CurveCAGDDlg dlg;

	sprintf(dlg._crvA.Buf(), "%.5f", _crvA);
	dlg._crvA.CalcLength();
	sprintf(dlg._crvB.Buf(), "%.5f", _crvB);
	dlg._crvB.CalcLength();
	sprintf(dlg._crvC.Buf(), "%.5f", _crvC);
	dlg._crvC.CalcLength();
	sprintf(dlg._crvD.Buf(), "%.5f", _crvD);
	dlg._crvD.CalcLength();

	sprintf(dlg._crvF.Buf(), "%.5f", _crvF);
	dlg._crvF.CalcLength();

	sprintf(dlg._crvT.Buf(), "%.5f", _crvT);
	dlg._crvT.CalcLength();

	sprintf(dlg._crvN.Buf(), "%i", _crvN);
	dlg._crvN.CalcLength();

	if ( dlg.DoModal(*this) == IDOK )
	{
		_crvA = static_cast<float>(atof(dlg._crvA));
		_crvB = static_cast<float>(atof(dlg._crvB));
		_crvC = static_cast<float>(atof(dlg._crvC));
		_crvD = static_cast<float>(atof(dlg._crvD));

		_crvF = static_cast<float>(atof(dlg._crvF));

		_crvT = static_cast<float>(atof(dlg._crvT));

		_crvN = (int)atof(dlg._crvN);

		_curveXYZView->mCInterpolation(_crvA, _crvB, _crvC, _crvD, _crvF, _crvT, _crvN);
	}
}


void CurveEdit::Modified()
{
	if (_pNotifySink->ContinuousMode())
	{
		ApplyNow();

		const char* pDot = strrchr(_curveXYZView->_filename, '.');
		if (0 != pDot)
		{
			if ( (_stricmp(pDot, ".s"  ) == 0) || 
				 (_stricmp(pDot, ".srf") == 0) )
			 _pNotifySink->SurfaceModified(false);
			else
			 _pNotifySink->CurveXYZModified(false);
		}
	}
}
