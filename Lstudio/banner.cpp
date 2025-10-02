#include <fw.h>

#include "banner.h"
#include "lstudioapp.h"
#include "lstudioptns.h"
#include "cmndefs.h"

#include "resource.h"

void Banner::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<Banner>::Proc);
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.Register();
}




HWND Banner::Create(HINSTANCE hInst, HWND hOwner)
{
	WinMaker wm(_ClassName(), hInst);
	wm.TopMost();
	wm.MakeBorder();
	wm.MakePopup(hOwner);
	wm.MakeVisible();
	return wm.Create();
}


Banner::Banner(HWND hwnd, const CREATESTRUCT* pCS) : 
Ctrl(hwnd, pCS), _Logo(pCS->hInstance, MAKEINTRESOURCE(IDB_DINO)),
_font(48, "Times New Roman MT Extra Bold")
{
	SIZE sz = { pWidth, pHeight };
	POINT center = { GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN) };
	center.x /= 2;
	center.y /= 2;
	RECT wndrect = { center.x - sz.cx/2, center.y - sz.cy/2, sz.cx + 4, sz.cy + 4 };
	MoveWindow(wndrect, false);
	if (PrjVar::IsEvalVer() && options.Expired())
		SetTimer(pTimerId, pDelay*3);
	else
		SetTimer(pTimerId, pDelay);
	_delay = 0;
	_canbeclosed = false;
}


Banner::~Banner()
{}


bool Banner::Paint()
{
	PaintCanvas cnv(Hwnd());
	_Logo.Draw(cnv, 1, 1);
	if (PrjVar::IsEvalVer() && options.Expired())
	{
		ObjectHolder sf(cnv, _font);
		cnv.BkMode(Canvas::bkTransparent);
		TextAlignment ta(cnv, TextAlignment::Center);
		cnv.TextColor(RGB(255, 255, 255));
		cnv.TextOut(pWidth/2, pHeight/2, "Expired demo version");
	}
	return true;
}


bool Banner::Timer(UINT id)
{
	if (PrjVar::IsEvalVer() && options.Expired())
	{
		UpdateCanvas cnv(Hwnd());
		ObjectHolder sf(cnv, _font);
		cnv.BkMode(Canvas::bkTransparent);
		TextAlignment ta(cnv, TextAlignment::Center);
		if (1 == _delay % 2)
			cnv.TextColor(RGB(255, 255, 255));
		else
			cnv.TextColor(RGB(0, 0, 0));
		cnv.TextOut(pWidth/2, pHeight/2, "Expired demo version");
	}

	switch (_delay)
	{
	case tDelay :
		{
			LStudioApp* pApp = dynamic_cast<LStudioApp*>(App::theApp);
			pApp->LaunchMainWindow();
			_canbeclosed = true;
		}
		break;
	case tCloseBanner :
		KillTimer(id);
		FORWARD_WM_CLOSE(Hwnd(), PostMessage);
		break;
	}
	_delay++;
	return true;
}


bool Banner::CanBeClosed()
{
	return _canbeclosed;
}
