#include <fw.h>
#include <glfw.h>

#include "drpdwnpalette.h"

HINSTANCE DropDownPalette::_hInst = 0;

COLORREF DropDownPalette::_Palette[] =
{
	RGB(0, 0, 0), RGB(0, 0, 255), RGB(0, 255, 0), RGB(0, 255, 255),
	RGB(255, 0, 0), RGB(255, 0, 255), RGB(255, 255, 0), RGB(255, 255, 255),	
	RGB(0, 0, 128), RGB(0, 128, 0), RGB(0, 128, 128), RGB(128, 0, 0),
	RGB(128, 0, 128), RGB(128, 128, 0), RGB(128, 128, 128),	RGB(128, 128, 255)
};

enum CellParams
{
	cpBorderSize = 4,
	cpCellWidth = 20,
	cpCellHeight = cpCellWidth
};

void DropDownPalette::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), WndProc);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE+1);
	wc.Register();
	_hInst = hInst;
}


DropDownPalette::DropDownPalette(HWND hParent, COLORREF rgb) :
_cb(0, 0, 0)
{
	_hParent = hParent;
	_rgb = rgb;
	_InitRect.left = 0;
	_InitRect.top = 0;
	_InitRect.right = eNumOfCols*(cpCellWidth+cpBorderSize)+3;
	_InitRect.bottom = (eNumOfRows+1)*(cpCellHeight+cpBorderSize)+4;
	AdjustWindowRect(&_InitRect, WS_POPUP | WS_VISIBLE | WS_DLGFRAME, false);
	RECT r;
	GetWindowRect(_hParent, &r);
	_InitRect.right -= _InitRect.left;
	_InitRect.left += r.left-_InitRect.left;
	_InitRect.bottom -= _InitRect.top;
	_InitRect.top += r.bottom-_InitRect.top;

	_current = eCustomColorId;
	for (int i=0; i<CountOf(_Palette); i++)
		if (rgb == _Palette[i])
			_current = i;
}


DropDownPalette::ChooseRetType DropDownPalette::Choose()
{
	_selected = false;
	_stay = true;
	_usecc = false;
	WinMaker maker(_ClassName(), _hInst);
	maker.MakePopup(_hParent);
	maker.MakeVisible();
	maker.MakeDlgFrame();
	maker.Origin(_InitRect.left, _InitRect.top);
	maker.Size(_InitRect.right, _InitRect.bottom);
	maker.lpData(this);
	_hWnd = maker.Create();
	ButtonMaker bmaker(_hInst);
	bmaker.MakeChild(1, _hWnd);
	bmaker.MakePushButton();
	bmaker.Origin(0, 4*(cpCellHeight+cpBorderSize)+cpBorderSize);
	bmaker.Size(3*(cpCellHeight+cpBorderSize), cpCellHeight+cpBorderSize-1);
	bmaker.Name(__TEXT("More..."));
	_hMore = bmaker.Create();
	HFONT hFont = GetWindowFont(GetParent(_hParent));
	if (0 != hFont)
		SetWindowFont(_hMore, hFont, false);
	ShowWindow(_hMore, SW_SHOW);
	SetFocus(_hWnd);
	while (_stay)
	{
		MSG msg;
		GetMessage(&msg, 0, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (_usecc)
	{
		return rtPickColor;
	}
	else if (_selected)
		return rtSelected;
	else
		return rtAborted;
}


LRESULT CALLBACK DropDownPalette::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DropDownPalette* pCtrl = GetWinLong<DropDownPalette*>(hWnd);
	switch (msg)
	{
	case WM_CREATE :
		{
			const CREATESTRUCT* pCS = reinterpret_cast<const CREATESTRUCT*>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCS->lpCreateParams));
		}
		return 0;
		HANDLE_MSG(hWnd, WM_PAINT, pCtrl->Paint);
		HANDLE_MSG(hWnd, WM_KILLFOCUS, pCtrl->KillFocus);
		HANDLE_MSG(hWnd, WM_CHAR, pCtrl->Char);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE, pCtrl->MouseMove);
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN, pCtrl->LButtonDown);
	case WM_COMMAND :
		pCtrl->Command(hWnd, LOWORD(wParam), Window(reinterpret_cast<HWND>(lParam)), HIWORD(wParam));
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}


void DropDownPalette::Command(HWND, int id, Window, UINT)
{
	switch (id)
	{
	case 1 :
		_ChooseColor();
		break;
	}
}


void DropDownPalette::_ChooseColor()
{
	_usecc = true;
}


void DropDownPalette::Paint(HWND) const
{
	PaintCanvas pc(_hWnd);
	int y = 0;
	int n = 0;
	int i = 0;
	for (i=0; i<eNumOfRows; i++)
	{
		int x = 0;
		for (int j=0; j<eNumOfCols; j++)
		{
			_DrawCell(pc, x, y, _Palette[n], n==_current);
			n++;
			x += cpBorderSize + cpCellWidth + 1;
		}
		y += cpBorderSize + cpCellHeight + 1;
	}
	assert(4==i);
	int x = 3*(cpBorderSize + cpCellWidth + 1);
	_DrawCell(pc, x, y, _rgb, eCustomColorId==_current);
}


void DropDownPalette::_DrawCell(Canvas& cnv, int x, int y, COLORREF rgb, bool focus) const
{
	if (focus)
		_DrawFocus(cnv, x, y, rgb);
	else
		_DrawFace(cnv, x, y, rgb);
}

void DropDownPalette::_DrawFace(Canvas& cnv, int x, int y, COLORREF rgb) const
{
	{
		ObjectHolder sp(cnv, pens3Dset.Shadow());
		cnv.MoveTo(x+cpCellWidth+2, y);
		cnv.LineTo(x, y);
		cnv.LineTo(x, y+cpCellHeight+4);
	}
	{
		ObjectHolder sp(cnv, pens3Dset.DkShadow());
		cnv.MoveTo(x+cpCellWidth+2, y+1);
		cnv.LineTo(x+1, y+1);
		cnv.LineTo(x+1, y+cpCellHeight+3);
	}
	{
		ObjectHolder sp(cnv, pens3Dset.Hilight());
		cnv.MoveTo(x+cpCellWidth+3, y);
		cnv.LineTo(x+cpCellWidth+3, y+cpCellHeight+3);
		cnv.LineTo(x, y+cpCellHeight+3);
	}
	{
		ObjectHolder sp(cnv, pens3Dset.Light());
		cnv.MoveTo(x+cpCellWidth+2, y+1);
		cnv.LineTo(x+cpCellWidth+2, y+cpCellHeight+2);
		cnv.LineTo(x+1, y+cpCellHeight+2);
	}
	RECT r = { x+2, y+2, x+cpCellWidth+2, y+cpCellHeight+2 };
	cnv.FilledRectangle(r, rgb);
}

void DropDownPalette::_DrawFocus(Canvas& cnv, int x, int y, COLORREF rgb) const
{
	{
		RECT r = { x+2, y+2, x+cpCellWidth+2, y+cpCellHeight+2 };
		cnv.FilledRectangle(r, rgb);
	}
	{
		ObjectHolder bp(cnv, GetStockObject(BLACK_PEN));
		cnv.MoveTo(x, y);
		cnv.LineTo(x+cpCellWidth+3, y);
		cnv.LineTo(x+cpCellWidth+3, y+cpCellHeight+3);
		cnv.LineTo(x, y+cpCellHeight+3);
		cnv.LineTo(x, y);

		cnv.MoveTo(x+2, y+2);
		cnv.LineTo(x+cpCellWidth+1, y+2);
		cnv.LineTo(x+cpCellWidth+1, y+cpCellHeight+1);
		cnv.LineTo(x+2, y+cpCellHeight+1);
		cnv.LineTo(x+2, y+2);
	}
	{
		ObjectHolder wp(cnv, GetStockObject(WHITE_PEN));

		cnv.MoveTo(x+1, y+1);
		cnv.LineTo(x+cpCellWidth+2, y+1);
		cnv.LineTo(x+cpCellWidth+2, y+cpCellHeight+2);
		cnv.LineTo(x+1, y+cpCellHeight+2);
		cnv.LineTo(x+1, y+1);
	}
}


void DropDownPalette::KillFocus(HWND, HWND hwndto)
{
	if (_hMore == hwndto)
		_ChooseColor();
	_stay = false;
	DestroyWindow(_hWnd);
}


void DropDownPalette::Char(HWND, TCHAR ch, int)
{
	switch (ch)
	{
	case 0x1B :
		CloseWindow(_hWnd);
		break;
	}
}

void DropDownPalette::LButtonDown(HWND, BOOL, int x, int y, UINT)
{
	int selected = _GetCell(x, y);
	if (selected != -1)
	{
		assert((selected>=0 && selected<ePalletteSize) || selected==eCustomColorId);
		if (eCustomColorId!=selected)
			_rgb = _Palette[selected];
		_selected = true;
		CloseWindow(_hWnd);
	}
}


void DropDownPalette::MouseMove(HWND, int x, int y, UINT)
{
	int crnt = _GetCell(x, y);
	if ((crnt != _current) && (crnt != -1))
	{
		assert((crnt>=0 && crnt<ePalletteSize) || crnt==eCustomColorId);
		UpdateCanvas cnv(_hWnd);
		_DrawFace(cnv, _current);
		_current = crnt;
		_DrawFocus(cnv, _current);
	}
}


int DropDownPalette::_GetCell(int x, int y) const
{
	int res = 0;
	int tx = cpBorderSize + cpCellWidth + 1;
	while (tx<x)
	{
		tx += cpBorderSize + cpCellWidth + 1;
		res++;
	}
	int ty = cpBorderSize + cpCellHeight + 1;
	while (ty<y)
	{
		ty += cpBorderSize + cpCellHeight + 1;
		res += eNumOfCols;
	}
	if (res>=ePalletteSize && res != eCustomColorId)
		res = -1;
	return res;
}

void DropDownPalette::_DrawFace(Canvas& cnv, int n) const
{
	assert((n>=0 && n<ePalletteSize) || n==eCustomColorId);
	int on = n;
	int x = 0;
	int y = 0;
	while (n>=eNumOfCols)
	{
		y += cpBorderSize + cpCellHeight + 1;
		n -= eNumOfCols;
	}
	while (n>0)
	{
		x += cpBorderSize + cpCellWidth + 1;
		n--;
	}
	_DrawFace(cnv, x, y, on != eCustomColorId ? _Palette[on] : _rgb);
}


void DropDownPalette::_DrawFocus(Canvas& cnv, int n) const
{
	assert((n>=0 && n<ePalletteSize) || n==eCustomColorId);
	int on = n;
	int x = 0;
	int y = 0;
	while (n>=eNumOfCols)
	{
		y += cpBorderSize + cpCellHeight + 1;
		n -= eNumOfCols;
	}
	while (n>0)
	{
		x += cpBorderSize + cpCellWidth + 1;
		n--;
	}
	_DrawFocus(cnv, x, y, on != eCustomColorId ? _Palette[on] : _rgb);
}
