#include <fstream>
#include <vector>

#include <fw.h>

#include "snapicon.h"
#include "params.h"
#include "menuids.h"
#include "writergb.h"

#include "resource.h"

const COLORREF SnapIcon::FrameColor = RGB(192, 0, 0);

SnapIcon* SnapIcon::Create(HWND hP, POINT p)
{
	GenCtrl::GWinMaker gwm(App::GetInstance());
	gwm.MakePopup(hP);
	gwm.MakeVisible();
	gwm.TopMost();
	const int thick = Params::Snap::nThickness;
	SIZE sz = { Params::Snap::nWidth+2*thick, Params::Snap::nHeight+2*thick };
	POINT origin = { p.x - sz.cx/2, p.y - sz.cy/2 };
	if (origin.x<0)
		origin.x = 0;
	if (origin.y<0)
		origin.y = 0;

	gwm.Size(sz);
	gwm.Origin(origin);
	SnapIcon* pRes = GenCtrl::Create<SnapIcon>(gwm);
	return pRes;
}


SnapIcon::SnapIcon(HWND hWnd, const GenCtrl::CreateData* pCD): 
Ctrl(hWnd, pCD->pCS()),
_pen(FrameColor), _hPrnt(pCD->GetParent()), 
_cnv(HWND(0)),
_snapMoveTask(this, _pen, _cnv),
_snapResizeTask(this, _pen, _cnv)
{
	_pTask = &_snapMoveTask;
}


SnapIcon::~SnapIcon()
{
	PostMessage(_hPrnt, WM_COMMAND, MAKEWPARAM(ID_UTILS_SNAPICONCLOSING, 0), 0);
}


bool SnapIcon::Size(SizeState, int, int)
{
	Region outside(0, 0, Width(), Height());
	const int thick = Params::Snap::nThickness;
	Region inside(thick, thick, Width()-thick, Height()-thick);
	Region r(outside, inside, RegionBase::eoDiff);
	_region.Swap(r);
	SetRegion(_region);
	return true;
}


bool SnapIcon::Paint()
{
	PaintCanvas pc(Hwnd(), FrameColor);
	return true;
}



bool SnapIcon::MouseMove(KeyState, int x, int y)
{
	_pTask->MouseMove(x, y);
	return true;
}



bool SnapIcon::LButtonDown(KeyState ks, int x, int y)
{
	SnapTask::eSide side;
	if (y<Params::Snap::nThickness)
		side = SnapTask::sTop;
	else if (y>Height()-Params::Snap::nThickness)
		side = SnapTask::sBottom;
	else if (x<Params::Snap::nThickness)
		side = SnapTask::sLeft;
	else
		side = SnapTask::sRight;

	if (ks.IsShift())
		_pTask = &_snapResizeTask;
	else
		_pTask = &_snapMoveTask;

	_pTask->LBDown(side, x, y);
	return true;
}

bool SnapIcon::LButtonUp(KeyState, int, int)
{
	_pTask->LBUp(0, 0);
	return true;
}

bool SnapIcon::CaptureChanged()
{
	_pTask->CaptureChanged();
	return true;
}

void SnapIcon::ContextMenu(HWND, UINT x, UINT y)
{
	MenuManipulator mm(App::theApp->GetContextMenu(SnapIconCMenu));
	mm.SetCheck(ID_SIZE_KEEPASPECT, _snapResizeTask.FixedAspect());
	TPMPARAMS tpm;
	tpm.cbSize = sizeof(TPMPARAMS);
	tpm.rcExclude.left = PosX();
	tpm.rcExclude.top = PosY();
	tpm.rcExclude.right = PosX()+Width();
	tpm.rcExclude.bottom = PosY()+Height();
	TrackPopupMenuEx
		(
		mm.Handle(), TPM_LEFTALIGN | TPM_TOPALIGN, 
		x, y, Hwnd(), &tpm
		);
}

bool SnapIcon::Command(int id, Window, UINT)
{
	switch (id)
	{
	case ID_SNAPICON_SNAPNOW :
		_SnapNow();
		break;
	case ID_SNAPICON_CLOSE :
		PostClose();
		break;
	case ID_SIZE_DEFAULTSIZE :
		_pTask = &_snapMoveTask;
		MoveWindow
			(
			PosX(), PosY(), 
			Params::Snap::nWidth+2*Params::Snap::nThickness, 
			Params::Snap::nHeight+2*Params::Snap::nThickness
			);
		break;
	case ID_SIZE_DOUBLESIZE :
		_pTask = &_snapMoveTask;
		MoveWindow
			(
			PosX(), PosY(), 
			2*Params::Snap::nWidth+2*Params::Snap::nThickness, 
			2*Params::Snap::nHeight+2*Params::Snap::nThickness
			);
		break;
	case ID_SIZE_KEEPASPECT :
		_KeepAspectRatio();
		break;
	}
	return true;
}


void SnapIcon::_SnapNow()
{
	UpdateCanvas src(HWND_DESKTOP);
	MemoryCanvas dst(src);
	const int thick = Params::Snap::nThickness;
	DDB bmp(src, Width()-2*thick, Height()-2*thick);
	{
		ObjectHolder sb(dst, bmp);
		BitBlt
			(
			dst, 
			0, 0, Width()-2*thick, Height()-2*thick, 
			src, 
			PosX()+thick, PosY()+thick, 
			SRCCOPY
			);
	}

	bmp.SaveRGB("icon", src);
}




void SnapIcon::_KeepAspectRatio()
{
	_snapResizeTask.ToggleFixedAspect();
}

void SnapIcon::_FreeSize()
{
	_pTask = &_snapResizeTask;
}
