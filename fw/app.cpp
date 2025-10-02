/**************************************************************************

  File:		app.cpp
  Created:	24-Nov-97


  Implementation of class App


**************************************************************************/


#include <assert.h>
#include <new.h>

#include <string>

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>

#include "warningset.h"

#include "mdimenus.h"
#include "exception.h"
#include "ownerdraw.h"
#include "window.h"
#include "app.h"
#include "sizestate.h"
#include "statusbar.h"
#include "mdiclient.h"
#include "keystate.h"
#include "ctrl.h"
#include "appctrl.h"
#include "mdictrl.h"


App* App::theApp = 0;
HINSTANCE App::_hDllInst = 0;


int OutOfMemory(size_t)
{
	throw Exception(__TEXT("Out of memory"));
}


App::App(HINSTANCE hInst, UINT menuId) : 
_pMain(0),
_hInst(hInst), 
_hAccel(0), _hModeless(0)
{
	theApp = this;
	_hDllInst = 0;
	_set_new_handler(OutOfMemory);
	InitCommonControls();
	GetVersionEx(&_os);
	if (menuId != 0)
		_contextMenus.Load(menuId);
}


App::~App()
{
	theApp = 0;
}



int App::Execute()
{
	MSG msg;

	while (GetMessage(&msg, 0, 0, 0))
	{
		if (0 == _hModeless || !IsDialogMessage(_hModeless, &msg))
			_TranslateAndDispatch(msg);
	}

	return static_cast<int>(msg.wParam);
}


void App::_TranslateAndDispatch(MSG& msg)
{
	TranslateMessage(&msg);
	DispatchMessage(&msg);
}

void App::_Show(Window::sw show)
{
	assert(0 != _pMain);
	_pMain->Show(show);
	_pMain->Update();
}


void App::ErrorBox(const Exception& e) const
{
	_pMain->ErrorBox(e);
}

void App::SetModeless(const Window& w)
{
	SetModeless(w.Hwnd());
}

MDIApp::MDIApp(HINSTANCE hInst, const MDIMenus::InitData* arr, int count, UINT contextMenuId) : 
App(hInst, contextMenuId),
_Menus(arr, count)
{
	_pMDIClient = 0;
}


int MDIApp::Execute()
{
	assert(0 != WMain());
	assert(0 != _pMDIClient);

	MSG msg;

	while (GetMessage(&msg, 0, 0, 0))
	{
		if (0 == GetModeless() || !IsDialogMessage(GetModeless(), &msg))
			_TranslateAndDispatch(msg);
	}

	return static_cast<int>(msg.wParam);
}


void MDIApp::_TranslateAndDispatch(MSG& msg)
{
	if (TranslateMDISysAccel(_pMDIClient->Hwnd(), &msg))
		return;
	if (_Accelerator() && TranslateAccelerator(WMain()->Hwnd(), _Accelerator(), &msg))
		return;
	TranslateMessage(&msg);
	DispatchMessage(&msg);
}


void MDIApp::_Show(Window::sw show)
{
	assert(0 != _pMDIClient);
	App::_Show(show);
}

void MDIApp::_PostCreate()
{
	assert(0 != WMain());

	MDICtrl* pCtrl = dynamic_cast<MDICtrl*>(WMain());
	assert(0 != pCtrl);
	_pMDIClient = pCtrl->MdiClient();
}


