/**************************************************************************

  File:		ctrl.cpp
  Created:	24-Nov-97


  Implementation of class Ctrl


**************************************************************************/


#include <cassert>

#include <string>

#include <windows.h>
#include <tchar.h>

#include "warningset.h"

#include "mdimenus.h"
#include "exception.h"
#include "menu.h"
#include "window.h"
#include "app.h"
#include "sizestate.h"
#include "statusbar.h"
#include "keystate.h"
#include "ownerdraw.h"
#include "ctrl.h"
#include "lstring.h"


Ctrl::Ctrl(HWND hwnd, const CREATESTRUCT* pCS) :
Window(hwnd)
{
	if (0 != pCS)
	{
		_size.cx = pCS->cx; _size.cy = pCS->cy;
		_pos.x = pCS->x; _pos.y = pCS->y;
	}
	else
	{
		_size.cx = 1; _size.cy;
		_pos.x = 0; _pos.y = 0;
	}
	SetWindowLongPtr(Hwnd(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

Ctrl::~Ctrl()
{
	SetWindowLongPtr(Hwnd(), GWLP_USERDATA, 0);
}


void Ctrl::OnDestroy(HWND hwnd)
{
	Ctrl* pSelf = reinterpret_cast<Ctrl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	delete pSelf;
}

