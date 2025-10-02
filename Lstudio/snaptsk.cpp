#include <fw.h>

#include "snapicon.h"
#include "params.h"

#include "resource.h"


const ROp::Rop SnapTask::mode = ROp::NotXorPen;


SnapMove::SnapMove(SnapIcon* pSnap, Pen& pen, Canvas& canvas) : 
SnapTask(pSnap, pen, canvas, LoadCursor(App::theApp->GetInstance(), MAKEINTRESOURCE(IDC_SNAP)))
{
	_prevpos.x = _prevpos.y = 0;
}

SnapResize::SnapResize(SnapIcon* pSnap, Pen& pen, Canvas& canvas) : 
SnapTask(pSnap, pen, canvas, LoadCursor(0, IDC_SIZEALL)), _keepAspect(true)
{}

void SnapMove::LBDown(eSide, int x, int y)
{
	_pSnap->GetWindowRect(_r);
	_prevpos.x = x; _prevpos.y = y;
	_capture = true;
	_pSnap->Capture();
}

void SnapResize::LBDown(eSide sd, int x, int y)
{
	_pSnap->GetWindowRect(_r);
	_side = sd;
	_capture = true;
	_prevpos.x = x; _prevpos.y = y;
	_pSnap->Capture();
}

void SnapMove::MouseMove(int x, int y)
{
	SetCursor(Cursor());
	if (_capture)
	{
		ROp rm(_cnv, mode);
		ObjectHolder sp(_cnv, _pen);
		DrawRubberband();
		int dx = x-_prevpos.x;
		int dy = y-_prevpos.y;
		MoveSnap(dx, dy);
		DrawRubberband();
		_prevpos.x = x;
		_prevpos.y = y;
	}
}

void SnapMove::MoveSnap(int dx, int dy)
{
	_r.left += dx;
	_r.right += dx;
	_r.top += dy;
	_r.bottom += dy;
}

void SnapResize::MouseMove(int x, int y)
{
	SetCursor(Cursor());
	if (_capture)
	{
		ROp rm(_cnv, mode);
		ObjectHolder sp(_cnv, _pen);
		DrawRubberband();
		int dx = x-_prevpos.x;
		int dy = y-_prevpos.y;

		if (ResizeSnap(_side, _keepAspect, dx, dy))
		{
			_prevpos.x = x;
			_prevpos.y = y;
		}
		DrawRubberband();
	}
}

void SnapTask::LBUp(int, int)
{
	if (_capture)
		ReleaseCapture();
}

void SnapTask::CaptureChanged()
{
	if (_capture)
	{
		ROp rm(_cnv, mode);
		ObjectHolder sp(_cnv, _pen);
		DrawRubberband();
		_capture = false;
		_pSnap->MoveWindow(_r.left, _r.top, _r.right-_r.left, _r.bottom-_r.top);
		_pSnap->Invalidate();
	}
}

bool SnapResize::ResizeSnap(eSide side, bool square, int dx, int dy)
{
	char bf[128];
	if (side==SnapTask::sTop)
	{
		int nsy = (_r.bottom-_r.top) - dy;
		if (nsy>=Params::Snap::nMinHeight)
		{
			_r.top += dy;
			_r.bottom = _r.top + nsy;
			if (square)
				_r.right = _r.left + nsy;;
			return true;
		}
	}
	else if (side == SnapTask::sBottom)
	{
		int nsy = (_r.bottom-_r.top) + dy;
		if (nsy>=Params::Snap::nMinHeight)
		{
			_r.bottom = _r.top + nsy;
			if (square)
				_r.right = _r.left + nsy;
			return true;
		}
	}
	else if (side == SnapTask::sLeft)
	{
		int nsx = (_r.right-_r.left) - dx;
		if (nsx>=Params::Snap::nMinWidth)
		{
			_r.left += dx;
			_r.right = _r.left + nsx;
			if (square)
				_r.bottom = _r.top + nsx;
			return true;
		}
	}
	else if (side == SnapTask::sRight)
	{
		int nsx = (_r.right-_r.left) + dx;
		sprintf(bf, "nsx = %d\n", nsx);
		OutputDebugString(bf);
		if (nsx>=Params::Snap::nMinWidth)
		{
			_r.right = _r.left + nsx;
			if (square)
				_r.bottom = _r.top + nsx;
			return true;
		}
	}

	return false;
}

void SnapTask::DrawRubberband() const
{
	POINT pts[5*Params::Snap::nThickness];
	DWORD idx[Params::Snap::nThickness];

	int ix = 0;
	for (int i=0; i<Params::Snap::nThickness; ++i)
	{
		idx[i] = 5;
		pts[ix].x = _r.left+i;
		pts[ix].y = _r.top+i;
		++ix;

		pts[ix].x = _r.right-1-i;
		pts[ix].y = pts[ix-1].y;
		++ix;

		pts[ix].x = pts[ix-1].x;
		pts[ix].y = _r.bottom-1-i;
		++ix;

		pts[ix].x = pts[ix-3].x;
		pts[ix].y = pts[ix-1].y;
		++ix;

		pts[ix] = pts[ix-4];
		++ix;
	}
	::PolyPolyline(_cnv, pts, idx, Params::Snap::nThickness);
}
