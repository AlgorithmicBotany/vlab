#include <string>

#include <fw.h>

#include "menuids.h"
#include "prjnotifysnk.h"


#include "objfgvobject.h"

#include "panelprms.h"
#include "panelctrl.h"
#include "paneldesign.h"
#include "panelitem.h"
#include "ped.h"

#include "resource.h"


const int HMargin = 20;
const int VMargin = 30;


void PanelCtrl::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<PanelCtrl>::Proc);
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PANEL));
	wc.Register();
}



PanelCtrl::PanelCtrl(HWND hWnd, const CREATESTRUCT* pCS) : 
Ctrl(hWnd, pCS),
_font((HFONT)PanelParameters::PanelFont.HObj()),
_timer(this, eTimerId, eTimerTimeout, false)
{
	_pDsgn = reinterpret_cast<PanelDesign*>(pCS->lpCreateParams);
	RECT r;
	_pDsgn->GetRect(r);

	_shift.x = HMargin - r.left;
	_shift.y = VMargin - r.top;

	SetText(_pDsgn->GetName());
	_directory = _pDsgn->GetDirectory();
	_dragging = false;
}


PanelCtrl::~PanelCtrl()
{
	_pDsgn->PanelClosed();
}


HWND PanelCtrl::Create(HINSTANCE hInst, PanelDesign* pDsgn)
{
	RECT r;
	pDsgn->GetRect(r);

	::InflateRect(&r, HMargin, VMargin);

	AdjustWindowRect
		(
		&r, 
		WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX | 
		WS_POPUP | WS_SYSMENU | WS_VISIBLE,
		false
		);

	{
		const int XShift = 20;
		const int YShift = 20;
		const int rshift = -r.left + XShift;
		const int dshift = -r.top + YShift;

		OffsetRect(&r, rshift, dshift);
	}

	WinMaker maker(_ClassName(), hInst);
	maker.MakeBorder();
	maker.MakeCaption();
	maker.MakeMinimizebox();
	maker.MakePopup(0);
	maker.TopMost();
	maker.MakeSysMenu();
	maker.MakeVisible();
	maker.Name(__TEXT("Panel"));
	maker.Origin(r.left, r.top);
	maker.Size(r.right-r.left, r.bottom-r.top);
	maker.lpData(pDsgn);
	return maker.Create();
}


bool PanelCtrl::Paint()
{
	PaintCanvas pc(Hwnd());
	pc.SetOrigin(_shift.x, _shift.y);
	ObjectHolder sf(pc, _font);
	_pDsgn->Draw(pc);

	return true;
}


bool PanelCtrl::LButtonDown(KeyState, int x, int y)
{
	UpdateCanvas cnv(Hwnd());
	cnv.SetOrigin(_shift.x, _shift.y);
	ObjectHolder sf(cnv, _font);
	RECT r = _pDsgn->ExecuteHit(cnv, x-_shift.x, y-_shift.y);
	
	if (r.left != r.right)
	{
		r.left += _shift.x;
		r.right += _shift.x;
		r.top += _shift.y;
		r.bottom += _shift.y;
		GrabFocus();
		MapWindowPoints(Hwnd(), 0, reinterpret_cast<POINT*>(&r), 2);
		ClipCursor(&r);
		_dragging = true;
	}
	return true;
}

bool PanelCtrl::LButtonUp(KeyState, int x, int y)
{
	if (_dragging)
	{
		UpdateCanvas cnv(Hwnd());
		cnv.SetOrigin(_shift.x, _shift.y);
		ObjectHolder sf(cnv, _font);
		_pDsgn->ExecuteDrag(cnv, x-_shift.x, y-_shift.y, true);
		ClipCursor(0);
		_dragging = false;
		_timer.Kill();
	}
	return true;
}

bool PanelCtrl::Timer(UINT id)
{
	if (id == _timer.Id())
	{
		UpdateCanvas cnv(Hwnd());
		cnv.SetOrigin(_shift.x, _shift.y);
		ObjectHolder sf(cnv, _font);
		if (_pDsgn->ExecuteDrag(cnv, _lp.x, _lp.y, false))
			_timer.Kill();
	}
	return true;
}

bool PanelCtrl::KillFocus(HWND)
{
	ClipCursor(0);
	_dragging = false;
	_timer.Kill();
	return true;
}


bool PanelCtrl::MouseMove(KeyState, int x, int y)
{
	if (_dragging)
	{
		_timer.Kill();
		_lp.x = x; _lp.y = y;
		UpdateCanvas cnv(Hwnd());
		cnv.SetOrigin(_shift.x, _shift.y);
		ObjectHolder sg(cnv, _font);
		if (!_pDsgn->ExecuteDrag(cnv, x-_shift.x, y-_shift.y, false))
			_timer.Start();
	}
	return true;
}


void PanelCtrl::ContextMenu(HWND, UINT x, UINT y)
{
	HMENU hMenu = App::theApp->GetContextMenu(PanelCtrlCMenu);
	_AdaptCMenu(hMenu);
	TrackPopupMenu(
		hMenu,
		TPM_LEFTALIGN | TPM_TOPALIGN,
		x, y,
		0,
		Hwnd(),
		0);
}



void PanelCtrl::_AdaptCMenu(HMENU hMenu)
{
	switch (_pDsgn->PanelTarget())
	{
	case PanelParameters::ptLsystem :
		EnableMenuItem(hMenu, ID_PANELCNTXT_NEWLSYSTEM, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, ID_PANELCNTXT_NEWVIEW, MF_BYCOMMAND | MF_GRAYED);
		break;
	case PanelParameters::ptViewFile :
		EnableMenuItem(hMenu, ID_PANELCNTXT_NEWLSYSTEM, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, ID_PANELCNTXT_NEWVIEW, MF_BYCOMMAND | MF_ENABLED);
		break;
	case PanelParameters::ptFile :
		EnableMenuItem(hMenu, ID_PANELCNTXT_NEWLSYSTEM, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hMenu, ID_PANELCNTXT_NEWVIEW, MF_BYCOMMAND | MF_ENABLED);
		break;
	}
	EnableMenuItem(hMenu, ID_PANELCNTXT_CLOSE, MF_BYCOMMAND | MF_ENABLED);
}


bool PanelCtrl::Command(int id, Window, UINT)
{
	switch (id)
	{
	case ID_PANELCNTXT_NEWMODEL :
		_pDsgn->NewModel(true);
		break;
	case ID_PANELCNTXT_NEWLSYSTEM :
		_pDsgn->NewLsystem(true);
		break;
	case ID_PANELCNTXT_NEWVIEW :
		_pDsgn->NewView(true);
		break;
	case ID_PANELCNTXT_CLOSE :
		PostMessage(Hwnd(), WM_CLOSE, 0, 0);
		break;
	case ID_PANELCNTXT_RESET :
		_Reset();
		break;
	case ID_PANELCNTXT_RERUN :
		_pDsgn->Rerun(true);
		break;
	}
	return true;
}


void PanelCtrl::_Reset()
{
	UpdateCanvas cnv(Hwnd());
	cnv.SetOrigin(_shift.x, _shift.y);
	ObjectHolder sf(cnv, _font);
	_pDsgn->DefaultValues(cnv);
}

