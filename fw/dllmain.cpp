/**************************************************************************

  File:		dllmain.cpp
  Created:	24-Nov-97


  Implementation of DllEntryPoint


**************************************************************************/


#include "warningset.h"

#include <assert.h>
#include <tchar.h>
#include <math.h>
#include <windows.h>

#include "window.h"
#include "ctrl.h"
#include "splitter.h"
#include "ucedit.h"
#include "canvas.h"
#include "colors.h"
#include "colorslider.h"
#include "huecircle.h"

#include "resource.h"
#include "dll.h"


HINSTANCE hDllInstance = 0;
HBITMAP hHue0128 = 0;
HBITMAP hHue0256 = 0;
HBITMAP hHue0512 = 0;


BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, void*)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH :
		hDllInstance = hInst;
		Splitter::Register(hInst);
		UCEdit::Register(hInst);
		ColorSlider::Register(hInst);
		HueCircle::Register(hInst);
		hHue0128 = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_HUE0128));
		hHue0256 = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_HUE0256));
		hHue0512 = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_HUE0512));
		break;
	case DLL_PROCESS_DETACH :
		UnregisterClass(UCEdit::_ClassName(), hInst);
		UnregisterClass(Splitter::_ClassName(), hInst);
		DeleteObject(hHue0512);
		DeleteObject(hHue0256);
		DeleteObject(hHue0128);
		UnregisterClass(ColorSlider::_ClassName(), hInst);
		UnregisterClass(HueCircle::_ClassName(), hInst);
		break;
	}
	return true;
}

