#include <cassert>

#include <string>

#include <windows.h>

#include "warningset.h"

#include "mdimenus.h"
#include "menu.h"
#include "exception.h"
#include "window.h"
#include "app.h"
#include "gdiobjs.h"

Cursor::Cursor(const char* nm)
{
	_hCursor = (HCURSOR)
		LoadImage(App::GetInstance(), nm, IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR);
	_shared = false;
}


Cursor::Cursor(dc name)
{
	_hCursor = LoadCursor(0, (const char*)name);
	_shared = true;
}


Cursor::~Cursor()
{
	if (!_shared)
		DestroyCursor(_hCursor);
}
