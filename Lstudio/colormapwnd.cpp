#include <fw.h>

#include "colormap.h"
#include "colormapwnd.h"
#include "menuids.h"
#include "prjnotifysnk.h"
#include "colormapedit.h"
#include "resource.h"

INIT_COUNTER(ColormapWnd);

extern std::string ExceptionLog;

void ColormapWnd::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<ColormapWnd>::Proc);
	wc.Register();
}

int ColormapWnd::GetDlgCode()
{
	return DLGC_WANTARROWS;
}

ColormapWnd::ColormapWnd(HWND hwnd, const CREATESTRUCT* pCS) : Ctrl(hwnd, pCS)
{
	_pEdit = 0;
	_firstActive = 0;
	_lastActive = 0;
	INC_COUNTER;
}


ColormapWnd::~ColormapWnd()
{
	DEC_COUNTER;
}


bool ColormapWnd::Size(SizeState, int w, int h)
{
	_cellW = (w-1)/eCols;
	_cellH = (h-1)/eRows;
	return true;
}



bool ColormapWnd::Paint()
{
	PaintCanvas pc(Hwnd());
	ObjectHolder sp(pc, GetStockObject(BLACK_PEN));
	POINT pt = { 0, 0 };
	int i = 0;
	for (int col = 0; col<eCols; col++)
	{
		pt.y = 0;
		for (int row = 0; row<eRows; row++)
		{
			RECT r = { pt.x, pt.y, pt.x+_cellW, pt.y+_cellH };
			pc.FilledRectangle(r, _map.Get(i));
			pc.Rectangle(r);
			pt.y += _cellH;
			i++;
		}
		pt.x += _cellW;
	}
	_MarkSelected(pc);
	return true;
}


void ColormapWnd::_MarkSelected(Canvas& cnv, bool white)
{
	ObjectHolder sp(cnv, GetStockObject(white ? WHITE_PEN : BLACK_PEN));
	POINT pt;
	for (int i=_firstActive; i<=_lastActive; i++)
	{
		_CellRect(i, pt);
		cnv.Rectangle(pt.x, pt.y, pt.x+_cellW, pt.y+_cellH);
	}
}


void ColormapWnd::_CellRect(int i, POINT& p)
{
	const int row = i / eCols;
	const int col = i % eRows;
	p.x = _cellW * row;
	p.y = _cellH * col;
}

void ColormapWnd::ContextMenu(HWND, UINT x, UINT y)
{
	HMENU hMenu = App::theApp->GetContextMenu(ColormapCMenu);
	MenuManipulator m(hMenu);
	if (_firstActive != _lastActive)
		m.Enable(ID_COLORMAPWND_REVERSE); // reverse
	else
		m.Disable(ID_COLORMAPWND_REVERSE); // reverse

	if (_firstActive+2<=_lastActive)
		m.Enable(ID_COLORMAPWND_INTERPOLATE); // smooth
	else
		m.Disable(ID_COLORMAPWND_INTERPOLATE); // smooth

	TrackPopupMenu(
		hMenu,
		TPM_LEFTALIGN | TPM_TOPALIGN,
		x, y,
		0,
		Hwnd(),
		0);
}

bool ColormapWnd::LButtonDown(KeyState ks, int x, int y)
{
	GrabFocus();
	if (ks.IsShift())
	{
		int selected = _CellFromPoint(x, y);
		if (-1 != selected)
		{
			if (selected<_lastActive)
				_SelectCell(selected, _lastActive);
			else if (selected>_firstActive)
				_SelectCell(_firstActive, selected);
		}
	}
	else
	{
		int newactive = _CellFromPoint(x, y);
		if (-1 != newactive)
			_SelectCell(newactive);
	}
	return true;
}


bool ColormapWnd::MouseMove(KeyState ks, int x, int y)
{
	if ((ks.IsLButton()) && (!(ks.IsShift())))
	{
		int curcell = _CellFromPoint(x, y);
		if (curcell<0 || curcell>=ePaletteSize)
			return true;

		if ((curcell != _firstActive) || (curcell != _lastActive))
			_SelectCell(curcell);
	}
	return true;
}


void ColormapWnd::_SelectCell(int firstactive, int lastactive)
{
	assert(firstactive>=0);
	assert(firstactive<ePaletteSize);

	if (-1 == lastactive)
		lastactive = firstactive;

	assert(lastactive>=0);
	assert(lastactive<ePaletteSize);

	UpdateCanvas cnv(Hwnd());
	_MarkSelected(cnv, false);
	_firstActive = firstactive;
	_lastActive = lastactive;
	_MarkSelected(cnv);

	_pEdit->SetCurrent(_firstActive);
	_pEdit->SetSliders(_map.Get(_firstActive));
}

int ColormapWnd::_CellFromPoint(int x, int y)
{
	assert(x>=0);
	assert(y>=0);

	const int col = x / _cellW;
	if (col>=eCols)
		return -1;
	const int row = y / _cellH;
	if (row>=eRows)
		return -1;
	return eRows*col + row;
}


bool ColormapWnd::Command(int id, Window, UINT)
{
	try
	{
		assert(0 != _pEdit);
		switch (id)
		{
		case ID_COLORMAPWND_INTERPOLATE :
			_Smooth();
			_pEdit->Modified(true);
			break;
		case ID_COLORMAPWND_INVERSE :
			_Inverse();
			_pEdit->SetSliders(_map.Get(_firstActive));
			_pEdit->Modified(true);
			break;
		case ID_COLORMAPWND_REVERSE :
			_Reverse();
			_pEdit->SetSliders(_map.Get(_firstActive));
			_pEdit->Modified(true);
			break;
		case ID_COLORMAPWND_APPLY :
			_pEdit->ApplyNow();
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


void ColormapWnd::SetNewRed(int red)
{
	assert(red>=0);
	assert(red<ePaletteSize);
	_map.SetR(_firstActive, red);
	_Repaint(_firstActive);
}


void ColormapWnd::SetNewGreen(int green)
{
	assert(green>=0);
	assert(green<ePaletteSize);
	_map.SetG(_firstActive, green);
	_Repaint(_firstActive);
}


void ColormapWnd::SetNewBlue(int blue)
{
	assert(blue>=0);
	assert(blue<ePaletteSize);
	_map.SetB(_firstActive, blue);
	_Repaint(_firstActive);
}


void ColormapWnd::_Repaint(int i)
{
	assert(i>=0);
	assert(i<ePaletteSize);
	UpdateCanvas cnv(Hwnd());
	POINT p;
	_CellRect(i, p);
	RECT r = { p.x+1, p.y+1, p.x+_cellW, p.y+_cellH };
	ObjectHolder sp(cnv, GetStockObject(_IsSelected(i) ? WHITE_PEN : BLACK_PEN));
	cnv.FilledRectangle(r, _map.Get(i));
}

bool ColormapWnd::KeyDown(UINT vk)
{
	switch (vk)
	{
	case VK_UP :
		{
			if (_firstActive>0)
				_SelectCell(_firstActive-1);
		}
		break;
	case VK_DOWN :
		{
			if (_firstActive<ePaletteSize-1)
				_SelectCell(_firstActive+1);
		}
		break;
	case VK_LEFT :
		{
			if (_firstActive>=eCols)
				_SelectCell(_firstActive-eCols);
		}
		break;
	case VK_RIGHT :
		{
			if (_firstActive<ePaletteSize-eCols)
				_SelectCell(_firstActive+eCols);
		}
		break;
	}
	return true;
}


void ColormapWnd::Import(const TCHAR* fname)
{
	ReadBinFile src(fname);

	BYTE rgb[3];
	COLORREF clr;

	for (int i=0; i<ePaletteSize; i++)
	{
		try
		{
			src.Read(rgb, sizeof(BYTE) * 3);
		}
		catch (Exception e)
		{
			ExceptionLog += e.Msg();
			ExceptionLog += __TEXT("\n");
			break;
		}
		clr = RGB(rgb[0], rgb[1], rgb[2]);
		_map.Set(i, clr);
	}
}


void ColormapWnd::Generate(WriteBinFile& trg) const
{
	BYTE rgb[3];
	COLORREF clr;
	for (int i=0; i<ePaletteSize; i++)
	{
		clr = _map.Get(i);
		rgb[0] = GetRValue(clr);
		rgb[1] = GetGValue(clr);
		rgb[2] = GetBValue(clr);
		trg.Write(rgb, sizeof(BYTE), 3);
	}
}
